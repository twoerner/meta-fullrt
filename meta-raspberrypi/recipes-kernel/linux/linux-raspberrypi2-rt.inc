inherit kernel siteinfo
require recipes-kernel/linux/linux-yocto.inc

DESCRIPTION = "Linux Kernel for Raspberry Pi 2 with RT-PREEMPT"
SECTION = "kernel"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

COMPATIBLE_MACHINE = "raspberrypi2"

PV = "${LINUX_VERSION}+git${SRCREV}"

# NOTE: For now we pull in the default config from the RPi kernel GIT tree.
KERNEL_DEFCONFIG_raspberrypi ?= "bcmrpi_defconfig"
KERNEL_DEFCONFIG_raspberrypi2 ?= "bcm2709_defconfig"

# CMDLINE for raspberrypi
CMDLINE = "dwc_otg.lpm_enable=0 console=ttyAMA0,115200 kgdboc=ttyAMA0,115200 root=/dev/mmcblk0p2 rootfstype=ext4 rootwait"

UDEV_GE_141 ?= "1"

# Set programmatically some variables during recipe parsing
# See http://www.yoctoproject.org/docs/current/bitbake-user-manual/bitbake-user-manual.html#anonymous-python-functions
python __anonymous () {
    kerneltype = d.getVar('KERNEL_IMAGETYPE', True)
    kerneldt = d.getVar('KERNEL_DEVICETREE', True)

    # Add dependency to 'rpi-mkimage-native' package only if RPi bootloader is used with DT-enable kernel
    if kerneldt:
        if kerneltype != 'uImage' and len(kerneldt.strip()) > 1:
            depends = d.getVar("DEPENDS", True)
            depends = "%s rpi-mkimage-native" % depends
            d.setVar("DEPENDS", depends)
}

do_kernel_configme_prepend() {
    install -m 0644 ${S}/arch/${ARCH}/configs/${KERNEL_DEFCONFIG} ${WORKDIR}/defconfig || die "No default configuration for ${MACHINE} / ${KERNEL_DEFCONFIG} available."
}

do_install_prepend() {
    install -d ${D}/lib/firmware
}

do_deploy_append() {
    # Deploy cmdline.txt
    install -d ${DEPLOYDIR}/bcm2835-bootfiles
    echo "${CMDLINE}" > ${DEPLOYDIR}/bcm2835-bootfiles/cmdline.txt
}

do_rpiboot_mkimage() {
    if test "x${KERNEL_IMAGETYPE}" != "xuImage" ; then
        if test -n "${KERNEL_DEVICETREE}"; then
            # Add RPi bootloader trailer to kernel image to enable DeviceTree support
            ${STAGING_DIR_NATIVE}/usr/lib/rpi-mkimage/mkknlimg --dtok ${KERNEL_OUTPUT} ${KERNEL_OUTPUT}
        fi
    fi
}
addtask rpiboot_mkimage before do_install after do_compile

# Enable OABI compat for people stuck with obsolete userspace
ARM_KEEP_OABI ?= "1"

# Quirk for udev greater or equal 141
UDEV_GE_141 ?= "1"

# Set the verbosity of kernel messages during runtime
# You can define CMDLINE_DEBUG in your local.conf or distro.conf to override this behaviour
CMDLINE_DEBUG ?= '${@base_conditional("DISTRO_TYPE", "release", "quiet", "debug", d)}'
CMDLINE_append = " ${CMDLINE_DEBUG}"

# Set a variable in .configure
# $1 - Configure variable to be set
# $2 - value [n/y/value]
kernel_configure_variable() {
    # Remove the config
    CONF_SED_SCRIPT="$CONF_SED_SCRIPT /CONFIG_$1[ =]/d;"
    if test "$2" = "n"
    then
        echo "# CONFIG_$1 is not set" >> ${B}/.config
    else
        echo "CONFIG_$1=$2" >> ${B}/.config
    fi
}

# Automatically depend on lzop-native if CONFIG_KERNEL_LZO is enabled
python () {
    try:
        defconfig = bb.fetch2.localpath('file://defconfig', d)
    except bb.fetch2.FetchError:
        return

    try:
        configfile = open(defconfig)
    except IOError:
        return

    if 'CONFIG_KERNEL_LZO=y\n' in configfile.readlines():
        depends = d.getVar('DEPENDS', False)
        d.setVar('DEPENDS', depends + ' lzop-native')
}
