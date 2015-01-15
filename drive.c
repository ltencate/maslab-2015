// Compile with:
// g++ test_motor.cpp -o test_motor -lmraa
// Controls a motor through a range of speeds using the Cytron motor controller
// Pwm on pin 9, and dir on pin 8.

#include <cassert>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>
#include <cmath>
#include <csignal>
#include <iostream>
#include <sys/time.h>
#include "mraa.hpp"
#include "gryo.h"
#include <pthread.h>
#include "mraa.hpp"

int running = 1;
int echo_state = 0;

void setMotorSpeed(mraa::Pwm& pwm, mraa::Gpio& dir, double speed) {
  assert(-1.0 <= speed && speed <= 1.0);
  if (speed < 0) {
    dir.write(1);
  }
  else {
    dir.write(0);
  }
  pwm.write(fabs(speed));
}

void sig_handler(int signo)
{
  if (signo == SIGINT) {
    printf("closing spi nicely\n");
    running = 0;
  }
}

static double distance = -1.0;
void echo_handler(void* args) {
  // Grab end time first, for accuracy
  if (echo_state == 1){echo_state =0;}
  else
    echo_state =1;
  struct timeval end;
  gettimeofday(&end, NULL);

  mraa::Gpio* echo = (mraa::Gpio*)args;
  static struct timeval start;
  bool rising = echo_state == 1;
  if (rising) {
    gettimeofday(&start, NULL);
  }
  else {
    int diffSec = end.tv_sec - start.tv_sec;
    // std::cout << "Diff sec: " << diffSec << std::endl;
    int diffUSec = end.tv_usec - start.tv_usec;
    //  std::cout << "Diff usec: " << diffUSec << std::endl;
     double diffTime = (double)diffSec + 0.000001*diffUSec;
    // std::cout << "Diff time: " << diffTime << std::endl;
    // Speed of sound conversion: 340m/s * 0.5 (round trip)
    std::cout << "Distance: " <<  diffTime * 170.0 << "m" << std::endl;
    distance = diffTime * 170.0;
  }
}

int main() {
  // Handle Ctrl-C quit
  pthread_t  gryo_thread = 0;
  pthread_create(&gryo_thread, NULL,&getTotal , NULL);
  signal(SIGINT, sig_handler);

  mraa::Pwm pwm = mraa::Pwm(9);
  pwm.write(0.0);
  pwm.enable(true);
  
  mraa::Pwm pwm1 = mraa::Pwm(3);
  pwm1.write(0.0);
  pwm1.enable(true);
  
  
  //assert(pwm != NULL);
  mraa::Gpio dir = mraa::Gpio(8);
  mraa::Gpio dir1 = mraa::Gpio(6);

  //assert(dir != NULL);
  dir.dir(mraa::DIR_OUT);
  dir.write(0);
  
  dir1.dir(mraa::DIR_OUT);
  dir1.write(0);
 
  mraa::Gpio trig = mraa::Gpio(2);
  trig.dir(mraa::DIR_OUT);
  mraa::Gpio echo = mraa::Gpio(4);
  echo.dir(mraa::DIR_IN);
  // Set the echo handlers to receive rising or falling edges of the
  // echo pulse
  echo.isr(mraa::EDGE_BOTH, echo_handler, &echo);  
 // double speed = -1.0;
  //  printf("Total: %f, Reading: %f, Time: %f\n",gyro.total, gyro.rf,gyro.-msf);
  while (running) {
   // std::cout << "Speed: " << speed << std::endl;
   /* double diff = 0-total;
    double integral += diff*dT;
    double derivative = reading;
    double power = P*diff + I*integral + D*derivative;
    double motorApwr = .4 + power;
    double motorB = .4 - power;*/ 
   echo_state = 0;
    trig.write(1);
    usleep(20);
    trig.write(0);
    if (distance > 0.2){
    setMotorSpeed(pwm, dir, .4);
    setMotorSpeed(pwm1, dir1, .4);
    }
    else
      {setMotorSpeed(pwm,dir,0);
	setMotorSpeed(pwm1,dir1,0);}
   // speed += 0.1;
   // if (speed > 1.0) {
    //  speed = -1.0;
      // Let the motor spin down
   //   setMotorSpeed(pwm, dir, 0.0);
   //   sleep(2.0);
  //  }
    usleep(200000);
  }
}
