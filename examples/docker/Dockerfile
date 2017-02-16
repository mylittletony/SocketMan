FROM debian:latest
MAINTAINER Chris Blake <chris@servernetworktech.com>

LABEL Description="This image is used as a build a copy of the Socketman binary for OpenWRT/LEDE."

# Create our own account to run under
ENV RUN_USER            builder
ENV RUN_GROUP           builder

# Install required packages
RUN \
  apt-get update \
  && apt-get install -y subversion build-essential libncurses5-dev zlib1g-dev \
  gawk git ccache gettext libssl-dev xsltproc zip wget mercurial gettext unzip \
  zlib1g-dev file python intltool libglib2.0-dev libnl-3-dev libnl-genl-3-dev \
  && useradd -m ${RUN_USER} \
  && mkdir -p /builddir/save

# Copy over our build script and targets
COPY ./files/build.sh /builddir

# Cleanup image
RUN \
  chown -R ${RUN_USER}:${RUN_GROUP} /builddir \
  && apt-get autoclean && apt-get --purge -y autoremove \
  && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Use our custom user for security purposes
USER ${RUN_USER}:${RUN_GROUP}

# Set our export dir as a volume so we can export our goods
VOLUME ["/builddir/save"]

# Set the default working directory
WORKDIR /builddir

# Let's start this build!
CMD ["./build.sh"]
