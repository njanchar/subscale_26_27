#include <stdio.h>
#include <stdlib.h> // Required for exit()
#include "i2c_driver.h"
#include "bmp581_lib.h"
#include <string.h>
#include "icm20948_lib.h"
#include <time.h>
#include <stdbool.h>
#include "drv8848.h"
#include <math.h>
#include <chrono>
#include <filesystem>
#include <poll.h>
#include <signal.h>
#include <sstream>
#include <sys/signalfd.h>
#include <sys/stat.h>

#include "/home/rocketpayload/subscale/rpicam-apps/core/frame_info.hpp"
#include "/home/rocketpayload/subscale/rpicam-apps/core/rpicam_app.hpp"
#include "/home/rocketpayload/subscale/rpicam-apps/core/still_options.hpp"

#include "/home/rocketpayload/subscale/rpicam-apps/output/output.hpp"

#include "/home/rocketpayload/subscale/rpicam-apps/image/image.hpp"

using namespace std::chrono_literals;
using namespace std::placeholders;
using libcamera::Stream;

namespace fs = std::filesystem;

//initalize the state machine types
typedef enum {
        PRE_FLIGHT,
        IN_FLIGHT,
        POST_FLIGHT
    } flight_state;

//intializing sensors
bmp581_data_t bmpData;
icm20948_data_t icmData;
icm20948_data_mag_t icmMagData;
timespec ts;

//init i2c bus
const char* i2c = "/dev/i2c-1";

//initalizing acceleration squared
float accelSq = 0.0f;

// timing variables
uint64_t startTime, launchStartTime, landingStartTime;

//main function
int main(void)
{

    //setup i2c connection
    i2cInit(&i2c);

    //intilialize the GPS module
	if(pa1010d_init(i2c, PA1010D_I2C_ADDR)<0){
		printf("GPS init failed!\n");
        return -1;

	}

    //intialize the icm20948
    if(icm20948_init(i2c, ICM20948_I2C_ADDR_PRIMARY)<0)
    {
        printf("ICM init failed!\n");
        return -1;
    }

    //intialize the bmp
	if(bmp581_init(i2c, BMP581_I2C_ADDR)<0){
		printf("BMP init failed!\n");
        return -1;

	}

    //initialze the motors
    drv8848Init();

    //intilaizs state to pre flight
    flight_state current_state = PRE_FLIGHT;

    //set all current motor states to zero, print out states to make sure
    cmdMotors(0,0,0,0);
	fprint("Motor states set to zero\n");

	//getting current time
    clock_gettime(CLOCK_MONOTONIC, &ts);
    // convert time to ms
    startTime = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;

    //initialize unique file name that does not exist in the directory to save the inflight data
    FILE *fptr;
    char *inflight_log_filename = get_unique_filename("inflight_log");
    // this data will go in the data folder

    //open inflight file so data can be stored there
    fptr = fopen(inflight_log_filename, "w");

	//write headers for file
	fprintf(log_file, "time,alt_m,press_pa,temp_c,ax,ay,az,gx,gy,gz,mx,my,mz\n");

    //check if the inflight log file was created successfully
	if (fptr == NULL) {
         fprintf(stderr, "Error creating inflight log file: %s\n", inflight_log_filename);
         return 1;
    }

    //intialize unique file name that does not exist in the directory to save the compute log data
    char *compute_log_filename = get_unique_filename("compute_log");

    //save initial altitude from BMP sensor to get offset so we can get difference from this
	bmp_read_sensor(bmpData);
	float init_alt = bmpData.altitude_m;

	//create currentAlt variable
	float current_alt = 0;

	//init current_time from laucnh start
	float current_time_from_launch = 0;

	//counters to track how many times the acceleration squared is greater than 4 and the change in altitude is greater than 20 meters from intial altitude
	//should have five consectuive true conditions before moving to inflight
	int preflight_thresholds_reached = 0;
	int inflight_thresholds_reached = 0;
	int previous_inflight_alt = 0;

	//defining booleans 
	bool mastExtended = false
	bool visionComplete = false
	bool post_flight_complete = false;

    //main loop that continually runs
    
    do{
        switch(current_state)
        {
            case PRE_FLIGHT:
            //get acceleration data from ICM20948
            icm20948_read_accel_gyro(i2c, ICM20948_I2C_ADDR_PRIMARY, &icmData);

            //get altitude data from BMP
			bmp_read_sensor(bmpData);
			current_alt = bmpData.altitude_m;

            //check to see if the acceleration squared is greater than 4 m/s^2 4Gs
            accelSq = icmData.accel_x_g^2 + icmData.accel_y_g^2 + icmData.accel_x_g^2;
            if(accelSq > 4*9.81 && (current_alt - init_alt)>20)
            {
				preflight_thresholds_reached++;
            }
			else{
				preflight_thresholds_reached = 0;
			}

            //if both conditions are met five times in a row, then change the state to IN_FLIGHT and save intial inflight start time
			if(preflight_thresholds_reached>=5){
                current_state = IN_FLIGHT;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                // convert time to ms
                launchStartTime = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
				//saving last altitude val
				previous_inflight_alt = current_alt;

			}

                break;
            case IN_FLIGHT:

                //get time from in flight start
				clock_gettime(CLOCK_MONOTONIC, &ts);
				current_time_from_launch = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL - launchStartTime;
                
				//read ICM20948 data
				icm20948_read_accel_gyro(i2c, ICM20948_I2C_ADDR_PRIMARY, &icmData);
				icm20948_read_mag(i2c, ICM20948_I2C_ADDR_PRIMARY, &icmMagData);
				
                //read BMP data
				bmp_read_sensor(bmpData);
				
                //save all data to file
				log_inflight_data(fptr, current_time_from_launch, &bmpData, &icmData, &icmMagData);

                //if the acceleration is less than 2 m/s^2 and the change in altitude is less than idk 5 meterrs over the last few samples, or it is has been like 5 mintues since the inflight
                //state started change the state to post flight
				current_alt = bmpData.altitude_m;
				accelSq = icmData.accel_x_g^2 + icmData.accel_y_g^2 + icmData.accel_x_g^2;

				if((current_alt - previous_inflight_alt) <5 && accelSq<2){
					inflight_thresholds_reached++;
				}
				else{
					inflight_thresholds_reached = 0;
				}

                //save intial post flight time
                clock_gettime(CLOCK_MONOTONIC, &ts);
                // convert time to ms
                uint64_t currentTime = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
         
                if(inflight_thresholds_reached == 5 || (currentTime-launchStartTime > 3*60*1000))
                {
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    // convert time to ms
                    landingStartTime = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
                    current_state = POST_FLIGHT;

					//flushes remaining inflight data, closes the inflight log, and opens the compute log
					print_inflight_data_to_file(fptr);
					fclose(fptr);
					fopen(fptr, compute_log_filename, "w");
                }
                break;
            case POST_FLIGHT:
                //check orientation and change it to make the mast stand straight up (when extended)
				orientation();

                //extend mast
				if (mastExtended == false){
					//move motor up
				}

				if (mastExtended == true && visionComplete == false){
					//run vision algorithm


					//at the end, vision complete is true
					visionComplete = true;

				}

				//if 
				if(mastExtended == true && visionComplete == true){
					//retract mast

					//after mast fully retracted
					post_flight_complete = true;
				}

				if (post_flight_complete == true){
					//turn all motors off
					fclose(file);
					while(1);
				}

                //close compute file 
                break;
            default:
                fprintf(stderr, "Unknown flight state\n");
                return 1;
        }

        // Need a delay here
    }while(1);

    return 0;
}

//vision algorithm, separete function in main?
void vision(){
    //save intial icm posiiton

    //take photo and rotate 70 degrees 5 times

    //run model on 5 images

    //compare image classification and confidence values

    //if image is found....
        //move motors to imu position
        //call function to take image
        //note final image file name or path

    //every step will be logged in the compute log, so print every step there. 

}

void orientation(){

}


// RPICAM HELPER FUNCTIONS DOWN HERE ---------------------------------------------------

class RPiCamStillApp : public RPiCamApp
{
public:
	RPiCamStillApp() : RPiCamApp(std::make_unique<StillOptions>())
	{
	}

	StillOptions *GetOptions() const
	{
		return static_cast<StillOptions *>(RPiCamApp::GetOptions());
	}
};

static std::string generate_filename(StillOptions const *options)
{
	std::string folder = options->Get().output; // sometimes "output" is used as a folder name
	if (!folder.empty() && folder.back() != '/')
		folder += "/";
	if (options->Get().datetime)
	{
		std::time_t raw_time;
		std::time(&raw_time);
		char time_string[32];
		std::tm *time_info = std::localtime(&raw_time);
		std::strftime(time_string, sizeof(time_string), "%m%d%H%M%S", time_info);
		std::ostringstream os;
		os << folder << time_string << "." << options->Get().encoding;
		return os.str();
	}
	else if (options->Get().timestamp)
	{
		std::ostringstream os;
		os << folder << (unsigned)time(NULL) << "." << options->Get().encoding;
		return os.str();
	}

	// The output string may contain a printf-style format specifier (e.g. "%04d") for the
	// frame number, so size the buffer dynamically to avoid truncating long filenames.
	std::string const &format = options->Get().output;
	int len = snprintf(nullptr, 0, format.c_str(), options->Get().framestart);
	if (len < 0)
		return {};
	std::string filename(len, '\0');
	snprintf(filename.data(), len + 1, format.c_str(), options->Get().framestart);
	return filename;
}

static void update_latest_link(std::string const &filename, StillOptions const *options)
{
	// Create a fixed-name link to the most recent output file, if requested.
	if (!options->Get().latest.empty())
	{
		fs::path link { options->Get().latest };
		fs::path target { filename };

		if (fs::exists(link) && !fs::remove(link))
			LOG_ERROR("WARNING: could not delete latest link " << options->Get().latest);
		else
		{
			fs::create_symlink(target, link);
			LOG(2, "Link " << options->Get().latest << " created");
		}
	}
}

static void save_image(RPiCamStillApp &app, CompletedRequestPtr &payload, Stream *stream, std::string const &filename)
{
	StillOptions const *options = app.GetOptions();
	StreamInfo info = app.GetStreamInfo(stream);
	BufferReadSync r(&app, payload->buffers[stream]);
	const std::vector<std::span<uint8_t>> mem = r.Get();
	if (stream == app.RawStream())
		dng_save(mem, info, payload->metadata, filename, app.CameraModel(), options);
	else if (options->Get().encoding == "jpg")
		jpeg_save(mem, info, payload->metadata, filename, app.CameraModel(), options);
	else if (options->Get().encoding == "png")
		png_save(mem, info, filename, options);
	else if (options->Get().encoding == "bmp")
		bmp_save(mem, info, filename, options);
	else
		yuv_save(mem, info, filename, options);
	LOG(2, "Saved image " << info.width << " x " << info.height << " to file " << filename);
}

static void save_images(RPiCamStillApp &app, CompletedRequestPtr &payload)
{
	StillOptions *options = app.GetOptions();
	std::string filename = generate_filename(options);
	save_image(app, payload, app.StillStream(), filename);
	update_latest_link(filename, options);
	if (options->Get().raw)
	{
		filename = filename.substr(0, filename.rfind('.')) + ".dng";
		save_image(app, payload, app.RawStream(), filename);
	}
	options->Set().framestart++;
	if (options->Get().wrap)
		options->Set().framestart %= options->Get().wrap;
}

static void save_metadata(StillOptions const *options, libcamera::ControlList &metadata)
{
	std::streambuf *buf = std::cout.rdbuf();
	std::ofstream of;
	const std::string &filename = options->Get().metadata;

	if (filename.compare("-"))
	{
		of.open(filename, std::ios::out);
		buf = of.rdbuf();
	}

	write_metadata(buf, options->Get().metadata_format, metadata, true);
}

// Some keypress/signal handling.

static int signal_received;
static void default_signal_handler(int signal_number)
{
	signal_received = signal_number;
	LOG(1, "Received signal " << signal_number);
}
static int get_key_or_signal(StillOptions const *options, pollfd p[1])
{
	int key = 0;
	if (options->Get().keypress)
	{
		poll(p, 1, 0);
		if (p[0].revents & POLLIN)
		{
			char *user_string = nullptr;
			size_t len;
			[[maybe_unused]] size_t r = getline(&user_string, &len, stdin);
			key = user_string[0];
		}
	}
	if (options->Get().signal)
	{
		if (signal_received == SIGUSR1)
			key = '\n';
		else if (signal_received == SIGUSR2)
			key = 'x';
		signal_received = 0;
	}
	return key;
}

// The main even loop for the application.

static void event_loop(RPiCamStillApp &app)
{
	StillOptions const *options = app.GetOptions();
	// output requested?
	bool output = !options->Get().output.empty() || options->Get().datetime || options->Get().timestamp;
	// "signal" mode is much like "keypress" mode
	bool keypress = options->Get().keypress || options->Get().signal;
	unsigned int still_flags = RPiCamApp::FLAG_STILL_NONE;
	if (options->Get().encoding == "rgb24" || options->Get().encoding == "png")
		still_flags |= RPiCamApp::FLAG_STILL_BGR;
	if (options->Get().encoding == "rgb48")
		still_flags |= RPiCamApp::FLAG_STILL_BGR48;
	else if (options->Get().encoding == "bmp")
		still_flags |= RPiCamApp::FLAG_STILL_RGB;
	if (options->Get().raw)
		still_flags |= RPiCamApp::FLAG_STILL_RAW;

	app.OpenCamera();

	// Monitoring for keypresses and signals.
	signal(SIGUSR1, default_signal_handler);
	signal(SIGUSR2, default_signal_handler);
	pollfd p[1] = { { STDIN_FILENO, POLLIN, 0 } };

	if (options->Get().immediate)
	{
		app.ConfigureStill(still_flags);
		while (keypress)
		{
			int key = get_key_or_signal(options, p);
			if (key == 'x' || key == 'X')
				return;
			else if (key == '\n')
				break;
			std::this_thread::sleep_for(10ms);
		}
	}
	else if (options->Get().zsl)
		app.ConfigureZsl(still_flags);
	else
		app.ConfigureViewfinder();
	app.StartCamera();
	auto start_time = std::chrono::high_resolution_clock::now();
	auto timelapse_time = start_time;
	int timelapse_frames = 0;
	constexpr int TIMELAPSE_MIN_FRAMES = 6; // at least this many preview frames between captures
	bool keypressed = false;
	enum
	{
		AF_WAIT_NONE,
		AF_WAIT_SCANNING,
		AF_WAIT_FINISHED
	} af_wait_state = AF_WAIT_NONE;
	int af_wait_timeout = 0;

	bool want_capture = options->Get().immediate;
	for (unsigned int count = 0;; count++)
	{
		RPiCamApp::Msg msg = app.Wait();
		if (msg.type == RPiCamApp::MsgType::Timeout)
		{
			LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
			app.StopCamera();
			app.StartCamera();
			continue;
		}
		if (msg.type == RPiCamApp::MsgType::Quit)
			return;
		else if (msg.type != RPiCamApp::MsgType::RequestComplete)
			throw std::runtime_error("unrecognised message!");

		CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);
		auto now = std::chrono::high_resolution_clock::now();
		int key = get_key_or_signal(options, p);
		if (key == 'x' || key == 'X')
			return;
		if (key == '\n')
			keypressed = true;

		// In viewfinder mode, run until the timeout or keypress. When that happens,
		// if the "--autofocus-on-capture" option was set, trigger an AF scan and wait
		// for it to complete. Then switch to capture mode if an output was requested.
		if (app.ViewfinderStream() && !want_capture)
		{
			LOG(2, "Viewfinder frame " << count);
			timelapse_frames++;

			bool timed_out = options->Get().timeout && (now - start_time) > options->Get().timeout.value;
			bool timelapse_timed_out = options->Get().timelapse &&
									   (now - timelapse_time) > options->Get().timelapse.value &&
									   timelapse_frames >= TIMELAPSE_MIN_FRAMES;

			if (af_wait_state != AF_WAIT_NONE)
			{
				FrameInfo fi(completed_request);
				bool scanning = fi.af_state == libcamera::controls::AfStateScanning;
				if (scanning || (af_wait_state == AF_WAIT_SCANNING && ++af_wait_timeout >= 16))
					af_wait_state = AF_WAIT_FINISHED;
				else if (af_wait_state == AF_WAIT_FINISHED)
					want_capture = true;
			}
			else if (timed_out || keypressed || timelapse_timed_out)
			{
				// Trigger a still capture, unless we timed out in timelapse or keypress mode
				if ((timed_out && options->Get().timelapse) || (!keypressed && keypress))
					return;

				keypressed = false;
				if (options->Get().af_on_capture)
				{
					libcamera::ControlList cl;
					cl.set(libcamera::controls::AfMode, libcamera::controls::AfModeAuto);
					cl.set(libcamera::controls::AfTrigger, libcamera::controls::AfTriggerStart);
					app.SetControls(cl);
					af_wait_state = AF_WAIT_SCANNING;
					af_wait_timeout = 0;
				}
				else
					want_capture = true;
			}
			if (want_capture)
			{
				if (!output)
					return;
				keypressed = false;
				af_wait_state = AF_WAIT_NONE;
				timelapse_time = std::chrono::high_resolution_clock::now();
				if (!options->Get().zsl)
				{
					app.StopCamera();
					app.Teardown();
					app.ConfigureStill(still_flags);
				}
				if (options->Get().af_on_capture)
				{
					libcamera::ControlList cl;
					cl.set(libcamera::controls::AfMode, libcamera::controls::AfModeAuto);
					cl.set(libcamera::controls::AfTrigger, libcamera::controls::AfTriggerCancel);
					app.SetControls(cl);
				}
				if (!options->Get().zsl)
					app.StartCamera();
			}
			else
				app.ShowPreview(completed_request, app.ViewfinderStream());
		}
		// In still capture mode, save a jpeg. Go back to viewfinder if in timelapse mode,
		// otherwise quit.
		else if (app.StillStream() && want_capture)
		{
			want_capture = false;
			if (!options->Get().zsl)
				app.StopCamera();
			LOG(1, "Still capture image received");
			save_images(app, completed_request);
			if (!options->Get().metadata.empty())
				save_metadata(options, completed_request->metadata);
			timelapse_frames = 0;
			if (!options->Get().immediate &&
				(options->Get().timelapse || options->Get().signal || options->Get().keypress))
			{
				if (!options->Get().zsl)
				{
					app.Teardown();
					app.ConfigureViewfinder();
				}
				if (options->Get().af_on_capture && options->Get().afMode_index == -1)
				{
					libcamera::ControlList cl;
					cl.set(libcamera::controls::AfMode, libcamera::controls::AfModeAuto);
					cl.set(libcamera::controls::AfTrigger, libcamera::controls::AfTriggerCancel);
					app.SetControls(cl);
				}
				if (!options->Get().zsl)
					app.StartCamera();
				af_wait_state = AF_WAIT_NONE;
			}
			else
				return;
		}
	}
}
