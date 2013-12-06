#include "capture.h"
#include "settings.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace cv;
using namespace std;

Capture::Capture():
	abort(false),
	pause(false),
	capture(0),
	readIndex(0),
	writeIndex(0)
{
	capture = new VideoCapture(CAM_ID);
    if ( !capture->isOpened() )
    {
		cerr << "No webcam" << endl;
		return;
    }
    
    captureThread = new thread(launcher, this);
}

Capture::~Capture()
{
	captureThread->join();
	delete captureThread;
}

int Capture::getImage(Mat & imageRef)
{
	if ((readIndex != writeIndex) || pause){
		imageRef = image[readIndex++];
		if (readIndex >= MAX_FRAMES) readIndex = 0;
		//TODO: add mutex here (for pause)
		if (pause) pause = false;		// unlock overrun condition
		return 0;
	} else {
		return -1;   	// readIndex == writeIndex means no new frames have been fetch from the camera
						// excepted for overrun condition.
	}
}

/*
 * Private stuff
 */
void Capture::launcher(void * instance){
	static_cast<Capture *>(instance)->run();
}

void Capture::run()
{
	chrono::milliseconds frameTime( 1000/FPS );
    
	cout << "Capture thread started..." << endl;
	
	while (!abort) {
		if ( !capture->isOpened() )
		{
			cerr << "No webcam" << endl;
			return;
		}
		
		if (!pause) {
			// update image
			capture->read(image[writeIndex++]);
			if (writeIndex >= MAX_FRAMES) writeIndex = 0;
			//TODO: add mutex here (for pause)
			if (writeIndex == readIndex) pause = true;		// overrun condition, pause until reader process the images
		}
		
		this_thread::sleep_for( frameTime );
	}
}
