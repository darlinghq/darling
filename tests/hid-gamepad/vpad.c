/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * vpad <xbox|ds4|joy>: a virtual game controller made through /dev/uinput.
 * "xbox" has the evdev layout of an Xbox One controller (buttons BTN_A to
 * BTN_THUMBR, 16-bit sticks, 10-bit triggers, hat), "ds4" that of a DualShock
 * 4 (8-bit axes, Bluetooth), "joy" a two-axis joystick with four buttons.
 * Commands on stdin: key CODE VALUE | abs CODE VALUE | sync | sleep MS | quit.
 * The device is removed when it quits.
 */
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static int fd;
static void ev(int t, int c, int v){ struct input_event e; memset(&e,0,sizeof e); e.type=t; e.code=c; e.value=v; write(fd,&e,sizeof e);}
static void ax(int code,int min,int max,int flat){ struct uinput_abs_setup s; memset(&s,0,sizeof s); s.code=code; s.absinfo.minimum=min; s.absinfo.maximum=max; s.absinfo.flat=flat; s.absinfo.fuzz=0; ioctl(fd,UI_ABS_SETUP,&s);}
int main(int argc,char**argv){
 fd=open("/dev/uinput",O_WRONLY|O_NONBLOCK); if(fd<0){perror("uinput");return 1;}
 const char*kind=argc>1?argv[1]:"xbox";
 struct uinput_setup us; memset(&us,0,sizeof us); us.id.bustype=BUS_USB;
 ioctl(fd,UI_SET_EVBIT,EV_KEY); ioctl(fd,UI_SET_EVBIT,EV_ABS);
 if(!strcmp(kind,"xbox")){
  strcpy(us.name,"Vpad Xbox One Controller"); us.id.vendor=0x045e; us.id.product=0x02ea; us.id.version=0x0408;
  int keys[]={BTN_A,BTN_B,BTN_X,BTN_Y,BTN_TL,BTN_TR,BTN_SELECT,BTN_START,BTN_MODE,BTN_THUMBL,BTN_THUMBR};
  for(unsigned i=0;i<sizeof keys/sizeof*keys;i++) ioctl(fd,UI_SET_KEYBIT,keys[i]);
  int axes[]={ABS_X,ABS_Y,ABS_RX,ABS_RY,ABS_Z,ABS_RZ,ABS_HAT0X,ABS_HAT0Y};
  for(unsigned i=0;i<8;i++) ioctl(fd,UI_SET_ABSBIT,axes[i]);
  ioctl(fd,UI_DEV_SETUP,&us);
  ax(ABS_X,-32768,32767,128);ax(ABS_Y,-32768,32767,128);ax(ABS_RX,-32768,32767,128);ax(ABS_RY,-32768,32767,128);
  ax(ABS_Z,0,1023,0);ax(ABS_RZ,0,1023,0);ax(ABS_HAT0X,-1,1,0);ax(ABS_HAT0Y,-1,1,0);
 } else if(!strcmp(kind,"ds4")){
  strcpy(us.name,"Vpad Wireless Controller"); us.id.vendor=0x054c; us.id.product=0x09cc; us.id.version=0x8111; us.id.bustype=BUS_BLUETOOTH;
  int keys[]={BTN_SOUTH,BTN_EAST,BTN_NORTH,BTN_WEST,BTN_TL,BTN_TR,BTN_TL2,BTN_TR2,BTN_SELECT,BTN_START,BTN_MODE,BTN_THUMBL,BTN_THUMBR};
  for(unsigned i=0;i<sizeof keys/sizeof*keys;i++) ioctl(fd,UI_SET_KEYBIT,keys[i]);
  int axes[]={ABS_X,ABS_Y,ABS_Z,ABS_RX,ABS_RY,ABS_RZ,ABS_HAT0X,ABS_HAT0Y};
  for(unsigned i=0;i<8;i++) ioctl(fd,UI_SET_ABSBIT,axes[i]);
  ioctl(fd,UI_DEV_SETUP,&us);
  ax(ABS_X,0,255,0);ax(ABS_Y,0,255,0);ax(ABS_Z,0,255,0);ax(ABS_RX,0,255,0);ax(ABS_RY,0,255,0);ax(ABS_RZ,0,255,0);ax(ABS_HAT0X,-1,1,0);ax(ABS_HAT0Y,-1,1,0);
 } else { /* generic joystick, no hat, one axis pair */
  strcpy(us.name,"Vpad Joystick"); us.id.vendor=0x1234; us.id.product=0x5678;
  for(int k=BTN_TRIGGER;k<BTN_TRIGGER+4;k++) ioctl(fd,UI_SET_KEYBIT,k);
  ioctl(fd,UI_SET_ABSBIT,ABS_X); ioctl(fd,UI_SET_ABSBIT,ABS_Y);
  ioctl(fd,UI_DEV_SETUP,&us); ax(ABS_X,-100,100,0);ax(ABS_Y,-100,100,0);
 }
 if(ioctl(fd,UI_DEV_CREATE)<0){perror("create");return 1;}
 printf("created\n"); fflush(stdout);
 char line[128]; int a,b;
 while(fgets(line,sizeof line,stdin)){
  if(sscanf(line,"key %i %i",&a,&b)==2) ev(EV_KEY,a,b);
  else if(sscanf(line,"abs %i %i",&a,&b)==2) ev(EV_ABS,a,b);
  else if(!strncmp(line,"sync",4)) ev(EV_SYN,SYN_REPORT,0);
  else if(sscanf(line,"sleep %i",&a)==1) usleep(a*1000);
  else if(!strncmp(line,"quit",4)) break;
 }
 ioctl(fd,UI_DEV_DESTROY); close(fd); return 0; }
