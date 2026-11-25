/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * vpad <xbox|ds4|joy|xboxff|ds4ff|ds4usbff|kbd|mouse>: a virtual game controller, keyboard or
 * mouse made through /dev/uinput. "xboxff" and "ds4ff" are "xbox" and "ds4" with
 * force feedback (FF_RUMBLE), "ds4usbff" is "ds4ff" on USB; "kbd" is a keyboard with LEDs and "mouse" a mouse with
 * three buttons, a wheel and a horizontal wheel. What the host sends to the device
 * (force-feedback uploads and plays, LED events) is printed on stdout:
 * "ff-upload strong=S weak=W length=L delay=D", "ff-play ID VALUE", "ff-erase ID",
 * "led CODE VALUE".
 * "xbox" has the evdev layout of an Xbox One controller (buttons BTN_A to
 * BTN_THUMBR, 16-bit sticks, 10-bit triggers, hat), "ds4" that of a DualShock
 * 4 (8-bit axes, Bluetooth), "joy" a two-axis joystick with four buttons.
 * Commands on stdin: key CODE VALUE | abs CODE VALUE | sync | sleep MS | quit.
 * The device is removed when it quits.
 */
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static int fd;
static void ev(int t, int c, int v){ struct input_event e; memset(&e,0,sizeof e); e.type=t; e.code=c; e.value=v; write(fd,&e,sizeof e);}
static void ax(int code,int min,int max,int flat){ struct uinput_abs_setup s; memset(&s,0,sizeof s); s.code=code; s.absinfo.minimum=min; s.absinfo.maximum=max; s.absinfo.flat=flat; s.absinfo.fuzz=0; ioctl(fd,UI_ABS_SETUP,&s);}
int main(int argc,char**argv){
 fd=open("/dev/uinput",O_RDWR|O_NONBLOCK); if(fd<0){perror("uinput");return 1;}
 const char*kind=argc>1?argv[1]:"xbox";
 int ff=0,usb=0; char base[16]; strncpy(base,kind,sizeof base-1); base[sizeof base-1]=0;
 if(!strcmp(kind,"xboxff")){strcpy(base,"xbox");ff=1;} else if(!strcmp(kind,"ds4ff")){strcpy(base,"ds4");ff=1;} else if(!strcmp(kind,"ds4usbff")){strcpy(base,"ds4");ff=1;usb=1;}
 kind=base;
 struct uinput_setup us; memset(&us,0,sizeof us); us.id.bustype=BUS_USB;
 ioctl(fd,UI_SET_EVBIT,EV_KEY); ioctl(fd,UI_SET_EVBIT,EV_ABS);
 if(!strcmp(kind,"xbox")||!strcmp(kind,"xboxbt")){
 ioctl(fd,UI_SET_EVBIT,EV_KEY);
 if(strcmp(kind,"kbd")&&strcmp(kind,"mouse")) ioctl(fd,UI_SET_EVBIT,EV_ABS);
 if(ff){ ioctl(fd,UI_SET_EVBIT,EV_FF); ioctl(fd,UI_SET_FFBIT,FF_RUMBLE); us.ff_effects_max=8; }
 if(!strcmp(kind,"kbd")){
  strcpy(us.name,"Vpad Keyboard"); us.id.vendor=0x046d; us.id.product=0xc31c; us.id.version=0x0110;
  for(int k=1;k<=121;k++) ioctl(fd,UI_SET_KEYBIT,k);
  ioctl(fd,UI_SET_EVBIT,EV_LED);
  for(int l=LED_NUML;l<=LED_KANA;l++) ioctl(fd,UI_SET_LEDBIT,l);
  ioctl(fd,UI_DEV_SETUP,&us);
 } else if(!strcmp(kind,"mouse")){
  strcpy(us.name,"Vpad Mouse"); us.id.vendor=0x046d; us.id.product=0xc077; us.id.version=0x0111;
  ioctl(fd,UI_SET_KEYBIT,BTN_LEFT); ioctl(fd,UI_SET_KEYBIT,BTN_RIGHT); ioctl(fd,UI_SET_KEYBIT,BTN_MIDDLE);
  ioctl(fd,UI_SET_EVBIT,EV_REL);
  int rel[]={REL_X,REL_Y,REL_WHEEL,REL_HWHEEL};
  for(unsigned i=0;i<4;i++) ioctl(fd,UI_SET_RELBIT,rel[i]);
  ioctl(fd,UI_DEV_SETUP,&us);
 } else if(!strcmp(kind,"xbox")){
  strcpy(us.name,"Vpad Xbox One Controller"); us.id.vendor=0x045e; us.id.product=0x02ea; us.id.version=0x0408;
  if(!strcmp(kind,"xboxbt")){ strcpy(us.name,"Xbox Wireless Controller"); us.id.product=0x02fd; us.id.version=0x0903; us.id.bustype=BUS_BLUETOOTH; }
  int keys[]={BTN_A,BTN_B,BTN_X,BTN_Y,BTN_TL,BTN_TR,BTN_SELECT,BTN_START,BTN_MODE,BTN_THUMBL,BTN_THUMBR};
  for(unsigned i=0;i<sizeof keys/sizeof*keys;i++) ioctl(fd,UI_SET_KEYBIT,keys[i]);
  int axes[]={ABS_X,ABS_Y,ABS_RX,ABS_RY,ABS_Z,ABS_RZ,ABS_HAT0X,ABS_HAT0Y};
  for(unsigned i=0;i<8;i++) ioctl(fd,UI_SET_ABSBIT,axes[i]);
  ioctl(fd,UI_DEV_SETUP,&us);
  ax(ABS_X,-32768,32767,128);ax(ABS_Y,-32768,32767,128);ax(ABS_RX,-32768,32767,128);ax(ABS_RY,-32768,32767,128);
  ax(ABS_Z,0,1023,0);ax(ABS_RZ,0,1023,0);ax(ABS_HAT0X,-1,1,0);ax(ABS_HAT0Y,-1,1,0);
 } else if(!strcmp(kind,"ds4")){
  strcpy(us.name,"Vpad Wireless Controller"); us.id.vendor=0x054c; us.id.product=0x09cc; us.id.version=0x8111; us.id.bustype=usb?BUS_USB:BUS_BLUETOOTH;
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
 setvbuf(stdin,NULL,_IONBF,0); /* select() on stdin must see what fgets() has not consumed */
 for(;;){
  fd_set rd; FD_ZERO(&rd); FD_SET(0,&rd); FD_SET(fd,&rd);
  if(select(fd+1,&rd,0,0,0)<0) break;
  if(FD_ISSET(fd,&rd)){
   struct input_event e;
   while(read(fd,&e,sizeof e)==sizeof e){
    if(e.type==EV_UINPUT&&e.code==UI_FF_UPLOAD){
     struct uinput_ff_upload up; memset(&up,0,sizeof up); up.request_id=e.value;
     ioctl(fd,UI_BEGIN_FF_UPLOAD,&up);
     printf("ff-upload strong=%u weak=%u length=%u delay=%u\n",up.effect.u.rumble.strong_magnitude,up.effect.u.rumble.weak_magnitude,up.effect.replay.length,up.effect.replay.delay);
     up.retval=0; ioctl(fd,UI_END_FF_UPLOAD,&up);
    } else if(e.type==EV_UINPUT&&e.code==UI_FF_ERASE){
     struct uinput_ff_erase er; memset(&er,0,sizeof er); er.request_id=e.value;
     ioctl(fd,UI_BEGIN_FF_ERASE,&er); printf("ff-erase %u\n",er.effect_id); er.retval=0; ioctl(fd,UI_END_FF_ERASE,&er);
    } else if(e.type==EV_FF) printf("ff-play %u %d\n",e.code,e.value);
    else if(e.type==EV_LED) printf("led %u %d\n",e.code,e.value);
    fflush(stdout);
   }
  }
  if(!FD_ISSET(0,&rd)) continue;
  if(!fgets(line,sizeof line,stdin)) break;
  {
  if(sscanf(line,"key %i %i",&a,&b)==2) ev(EV_KEY,a,b);
  else if(sscanf(line,"abs %i %i",&a,&b)==2) ev(EV_ABS,a,b);
  else if(sscanf(line,"rel %i %i",&a,&b)==2) ev(EV_REL,a,b);
  else if(!strncmp(line,"sync",4)) ev(EV_SYN,SYN_REPORT,0);
  else if(sscanf(line,"sleep %i",&a)==1) usleep(a*1000);
  else if(!strncmp(line,"quit",4)) break;
  }
 }
 ioctl(fd,UI_DEV_DESTROY); close(fd); return 0; }
