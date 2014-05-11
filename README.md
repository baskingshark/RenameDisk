RenameDisk
==========
An OS X kernel extension to rename disks.

Purpose
-------
Apple only enables TRIM support for Apple branded SSDs.  This kext changes the
name of the disk from **FOO** to **APPLE SSD (FOO)**.  This, by itself, it not
enough to enable to TRIM support as when this kext gets loaded, the
IOAHCIBlockStorageDriver/AppleAHCIDiskDriver has already determined that TRIM
support is not available.  By stopping and restarting the driver, the system
can be fooled into enabling TRIM support.

See Also
--------
Any of the many resources on the Internet that modify the existing Apple driver
to achieve the same result.  For example,

* http://www.mactrast.com/2011/07/how-to-enable-trim-support-for-all-ssds-in-os-x-lion/
* http://chameleon.alessandroboschini.it/index.php
* http://www.cindori.org/software/trimenabler/

License
------
    Copyright (c) 2014, baskingshark
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
