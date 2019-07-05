#include <usb.h>
#include <stdio.h>
#include <string.h>

#define VERSION "0.1.0"
#define VENDOR_ID 0x045E
#define PRODUCT_ID 0x028E
#define INTERFACE 0

static const int reqIntLen=20;
static const int endpoint_Int_in=0x81; /* endpoint 0x81 address for IN */
static const int endpoint_Int_out=0x01; /* endpoint 1 address for OUT */

static const int timeout=2000; /* timeout in ms */
 
void bad(const char *why) {
     fprintf(stderr,"Fatal error> %s\n",why);
     exit(17);
}

usb_dev_handle *find_lvr_hid();

usb_dev_handle* setup_libusb_access() 
{
  usb_dev_handle *lvr_hid;
  int retval;
  char dname[32] = {0};
  usb_set_debug(255);
  usb_init();
  usb_find_busses();
  usb_find_devices();

  if(!(lvr_hid = find_lvr_hid())) {
       printf("Couldn't find the USB device, Exiting\n");
       return NULL;
   }
 
  printf("usb_get_driver_np\n");
  retval = usb_get_driver_np(lvr_hid, 0, dname, 31);
  printf("usb_get_driver_np error is : %d \r\n",retval);
  if (!retval){
       retval = usb_detach_kernel_driver_np(lvr_hid, 0);
       printf("usb_detach_kernel_driver_np is : %d \r\n",retval);
  }
  printf("dname is : %s\n",dname);
 
  retval=usb_set_configuration(lvr_hid, 1);
  if ( retval < 0) {
       printf("Could not set configuration 1 : %d\n", retval);
       return NULL;
  }
  retval=usb_claim_interface(lvr_hid, INTERFACE);
  if ( retval < 0) {
       printf("Could not claim interface: %d\n", retval);
       return NULL;
  }
  return lvr_hid;
}
 
usb_dev_handle * find_lvr_hid() 
{
  struct usb_bus *bus;
  struct usb_device *dev;

  for (bus = usb_get_busses(); bus; bus = bus->next){
      for (dev = bus->devices; dev; dev = dev->next) {
          //printf("Next hid with Vendor Id: %x and Product Id: %x.\n", dev->descriptor.idVendor, dev->descriptor.idProduct);
          if (dev->descriptor.idVendor == VENDOR_ID &&  dev->descriptor.idProduct == PRODUCT_ID ) {
             usb_dev_handle *handle;
             printf("lvr_hid with Vendor Id: %x and Product Id: %x found.\n", VENDOR_ID, PRODUCT_ID);
             if (!(handle = usb_open(dev))) {
                    printf("Could not open USB device\n");
                    return NULL;
               }
               return handle;
          }
     }
 }
 return NULL;
}
 
void Write_RumbleData(usb_dev_handle *dev,uint8_t left,uint8_t right)
{
     int r;
     //uint8_t buffer[8];
     char buffer[8];
     memset(buffer,0,sizeof(buffer));
     buffer[1] = 0x08;
     buffer[3] = left;
     buffer[4] = right;
     r = usb_interrupt_write(dev, endpoint_Int_out, buffer, 8, timeout);
     if( r < 0 ){
          perror("USB interrupt write"); bad("USB write failed"); 
     }
}

void Get_VoltagePercent(usb_dev_handle *dev)
{
     char read_data[20];
     int r,i;
     memset(read_data,0,sizeof(read_data));
     Write_RumbleData(dev,0x01,0x00);
     Write_RumbleData(dev,0x00,0x01);
     Write_RumbleData(dev,0x01,0x01);
     Write_RumbleData(dev,0x00,0x00);
//     do{
          r = usb_interrupt_read(dev, endpoint_Int_in, read_data, reqIntLen, timeout);
          if( r != reqIntLen ){
               perror("USB interrupt read"); bad("USB read failed"); 
          }else{
               printf( "Read data: " );
               for(i=0;i<reqIntLen;i++)
               {
                    printf("%02x ",read_data[i]);
               }
               printf( "\r\n" );
               
               if( read_data[4] & 0x80 ){
                    printf( "The GPi Case power via DC Adapter.\r\n" );
               }else{
                    printf("The voltage percent(0-0x64) is : %02x \r\n",read_data[4]);
               }
          }
          
//          if((read_data[2]&0x30) == 0x30){//break if press SELECT and START button
//               printf( "It's break if press ELECT and START buttons.\r\n" );
//               break;
//          }
//     }while(1);
}
 
//int main( int argc, char **argv)
int main()
{
     usb_dev_handle *lvr_hid;
     int ret=0;
     if ((lvr_hid = setup_libusb_access()) == NULL) {
          exit(-1);
     } 

     Get_VoltagePercent(lvr_hid);
     //ret = usb_clear_halt(lvr_hid, unsigned int ep);
     ret = usb_reset(lvr_hid);
     printf("usb_reset error is : %d \r\n",ret);
     ret = usb_release_interface(lvr_hid, INTERFACE);
     printf("usb_release_interface error is : %d \r\n",ret);
     ret = usb_close(lvr_hid);
     printf("usb_close error is : %d \r\n",ret);

     return ret;
}
