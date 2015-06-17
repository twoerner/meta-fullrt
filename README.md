# -rt patches

This OpenEmbedded layer adds support for building Linux kernels for some
devices using the full -rt patch from
https://rt.wiki.kernel.org/index.php/Main_Page.

As an example, to build the RaspberryPi2's 3.18.11 kernel using that
series' latest -rt patch (-rt7) add this layer to your build and, in your
conf/local.conf add:

```shell
PREFERRED_PROVIDER_virtual/kernel_forcevariable = "linux-raspberrypi2-rt"
```
