

#include <stdio.h>
#include "miosix.h"
#include "interfaces-impl/rtc.h"
#include "interfaces-impl/gpio_timer.h"
#include "interfaces-impl/power_manager.h"
#include "interfaces-impl/high_resolution_timer_base.h"
using namespace std;
using namespace miosix;

/**
 * Get tick in low frequency and high frequency to check if the are really coherent
 * @param n number of time to test it
 * @param forceReturnOnError true means exit from function on first error found
 */
void testTickCorrectness(int n, bool verbose=0, bool forceReturnOnError=0){
    long long tickH,tickL;
    double timeH,timeL;
    Rtc& rtc=Rtc::instance();
    GPIOtimer& timer=GPIOtimer::instance();
    
    for(int i=0;i<n;i++){
	//Fragment of code to check if HRt is consistent with RTC. 
	//On long running without resync, this code will fail due to the accumulating skew
	tickH=timer.getValue();
	tickL=rtc.getValue();
	timeH=(double)(tickH+HRTB::clockCorrection)/48000000;
	timeL=(double)tickL/32768;
	bool error=timeH-timeL>0.000032;
	if(error){
	    printf("Sync error --> \t\t");
	}
	if(verbose || error){
	    printf("%lld %.9f %lld %lld %.9f %.9f", 
		tickL,
		timeL,
		HRTB::clockCorrection,
		tickH,
		timeH,
		timeH-timeL);
	}
	if(verbose || error){
	    printf("\n");
	}
	if(error && forceReturnOnError){
	    return ;
	}
    }
}

int main(int argc, char** argv) {
    printf("Test\n");
    Rtc& rtc=Rtc::instance();
    PowerManager& pm=PowerManager::instance();
    for(long long i=800000;;i+=800000){
	Thread::sleep(10000);
	HRTB::stopResyncSoft();
	printf("Start lock..");
	fflush(stdout);
	Thread::sleep(4000);
	printf("End lock\n");
	HRTB::startResyncSoft();
	Thread::sleep(7500);
	printf("[%lld] Sleep until %lld\n",rtc.getValue(),i);
	pm.deepSleepUntil(i);
	
	printf("[%lld] Wake up!\n\n",rtc.getValue());
    }
    
    return 0;
}

