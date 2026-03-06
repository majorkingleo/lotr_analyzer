FROM debian:trixie
RUN apt update -y 
RUN apt-get --yes install \
	libzstd1 \
	libmimetic0t64 \
	default-mysql-client-core \
	locales \
	locales-all 

RUN	locale-gen "en_US.UTF-8"
RUN update-locale "LANG=en_US.UTF-8"
RUN locale-gen --purge "en_US.UTF-8"
RUN dpkg-reconfigure --frontend noninteractive locales

CMD /bin/bash



