Cory Quammen <cquammen@cs.unc.edu>
Last modified: 2013-02-01

Using the 3DConnexion Space Navigator with the MADAI Workbench

The Space Navigator from 3DConnexion is a 6 degree-of-freedom input
device that can be used to manipulate the currently active view in the
MADAI Workbench. To use the Space Navigator on your system, follow the
instructions below.

Mac OS X

Simply plug in the Space Navigator to an available USB port. No
further action should be required.

Linux

You will need to make possibly two modifications:

1). Install a policy for the Hardware Abstraction Layer (HAL). If you
    plugin in the Space Navigator and it manipulates the mouse, you
    will need to tell X not to treat it as a mouse. To do this,
    create a text file at /etc/hal/fdi/policy/ named
    disable-space-navigator-as-mouse.fdi. In it, add the contents

<?xml version="1.0" encoding="ISO-8859-1"?>
<deviceinfo version="0.2">
 <device>
   <match key="info.product" contains="SpaceNavigator">
     <remove key="input.x11_driver"/>
   </match>
 </device>
</deviceinfo>

    Restart X and check to see that the Space Navigator no longer
    moves the mouse cursor.

2). Install a udev rule that gives the MADAI Workbench permission to
    read and write to the Space Navigator's device file. Create a text file at
    /etc/udev/rules.d named 10.space-navigator.rules. In it, add the contents

SUBSYSTEM=="usb", ATTRS{idVendor}=="046d", ATTRS{idProduct}=="c626", RUN+="/bin/bash -c 'chmod 666 /dev/$name'"

    There is no need to escape the single and double quotation
    marks. This rule tells the system to give read/write permission
    for just the Space Navigator to everyone on the system. This is
    required to avoid needing to run the MADAI Workbench as a
    superuser/root.
