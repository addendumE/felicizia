#include <stdio.h>
#include <unistd.h>
#include "MainApp.h"

int main(int argc, char **argv)
{
  MainApp *app = new MainApp;
	app->start();
	  while(1) {
	      /* Periodically call the lv_task handler.
	       * It could be done in a timer interrupt or an OS task too.*/
	      //lv_timer_handler();
	      sleep(1);
	  }
}

