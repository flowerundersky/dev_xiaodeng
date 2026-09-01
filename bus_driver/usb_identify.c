#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h> 

#define USB_INDENTIFY_VENDOR_ID 0x1234   //供应商
#define USB_INDENTIFY_PRODUCT_ID 0x5678

struct usb_indentify {
    struct usb_device *udev;
    struct usb_interface *interface;

};

/*
 * 设备匹配表：
 * 将 USB_IDENTIFY_VENDOR_ID 和 USB_IDENTIFY_PRODUCT_ID
 * 替换成目标设备的 VID 和 PID。
 */

static const struct usb_device_id usb_indentify_table[]={
    {
        USB_DEVICE(
            USB_INDENTIFY_PRODUCT_ID,
            USB_INDENTIFY_VENDOR_ID
        )
    },

    {}
}

MODULE_DEVICE_TABLE(usb,usb_indentify_table);

/*
 * 设备插入、驱动加载或重新绑定时调用。
 */
static  int usb_indenty_probe(
    struct usb_interface *interface ,
    const struct usb_device_id *id
){
    struct usb_device *udev;
    struct usb_host_interface *iface_desc; //desc:description 
    struct usb_endpiont_descriptor *endpoint;
    struct usb_indentify_dev *dev;
    int i;
    udev=interface_to_usbdev(interface);
    iface_desc=interface->cur_altsetting; //alternate:轮询 ，指向正在运行的设备

    dev=kzalloc(sizeof(*dev),GFP_KERNEL); //kernel zero allocattion  
    if(!dev)
    return -ENOMEN;

        dev->udev=udev;
    dev->interface=interface;

    dev_set_intfdata(interface,dev);

    dev_info(&interface->dev,
            "USB device detected: VID=%04X, PID=%04X\n",
            le16_to_cpu(udev->descriptor.idVendor),
            le16_to_cpu(udev->descriptor.idProduct)
    );

    dev_info(&interface->dev,
            "USB speed : %s\n",
            usb_speed_string(udev->speed)
    );

    dev_info(&interface->dev,
            "USB class:0x%02X, subclass:0x%02X, protocol:0x%02X\n",
            iface_desc->desc.bInterfaceClass,
            iface_desc->desc.bInterfaceSubClass,
            iface_desc->desc.bInterfaceProtocol
    );

    dev_info(&interface->dev,
            "USB number of interfaces: %u\n",
            iface_desc->desc.bInterfaceNumber
    );


    /*
    遍历当前接口的所有节点
    */

    for(i=0 ; i<iface_desc->desc.bNumEndpoints ; i++)
    {
        endpoint= &iface_desc-> [i].desc;
        dev_info(&interface->dev,
                "Endpoint %u: address=0x%02X, attributes=0x%02X, max_packet=%u\n",
                i,
                endpoint->bEndpointAddress,
                endpoint->bmAttributes,
                le16_to_cpu(endpoint->wMaxPacketSize)
        );
        if(usb_endpoint_is_bulk_in(endpoint))
        {
            dev_info(&interface->dev,
                    "type: BULK IN\n",
            );
        }
        else if(usb_endpoint_is_bulk_out(endpoint))
        {
            dev_info(&interface->dev,
                    "type: BULK out\n",
            );
        }
        else if(usb_endpoint_xfer_int(endpoint))
        {
            dev_info(&interface->dev,
                    "type: INTERRUPT\n"
            )
        }
        else if(usb_endpoint_xfer_isoc(endpoint))
        {
            dev_info(&interface->dev,
                    "type: ISOCHRONOUS\n"
            )
        }
        else if(usb_endpoint_xfer_control(endpoint))
        {
            dev_info(&interface->dev,
                    "type: CONTROL\n"
            )
        }
        else
        {
            dev_info(&interface->dev,
                    "type: UNKNOWN\n"
            )
        }


    }
    return 0;
}

/*
 * 设备拔出、驱动卸载或重新绑定时调用。
 */
static void usb_indentify_disconnect(struct usb_interface *interface)
{
    struct usb_indentify_dev *dev ;
    dev= usb_get_intfdata(interface);
    usb_set_intfdata(interface, NULL);

    if (dev) {
        dev_info(&interface->dev, "USB device disconnected\n");
        kfree(dev);
    }
}

static struct usb_driver usb_indentify_driver = {
    .name = "usb_indentify",
    .id_table = usb_indentify_table,
    .probe = usb_indentify_probe,
    .disconnect = usb_indentify_disconnect,
};

module_usb_driver(usb_indentify_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("USB Identify Driver");
MOUDULE_VERSION("1.0");
