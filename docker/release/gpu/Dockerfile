# Build stage with Spack pre-installed and ready to be used
FROM spack/ubuntu-bionic:latest as builder
# FROM ecpe4s/ubuntu18.04-spack-x86_64:0.15.3 as builder

ARG CUDA_ARCH=60

# What we want to install and how we want to install it
# is specified in a manifest file (spack.yaml)
RUN mkdir /opt/timemory \
    &&  (echo "spack:" \
    &&   echo "  specs:" \
    &&   echo "  - cmake" \
    &&   echo "  - cuda" \
    &&   echo "  - nccl cuda_arch=${CUDA_ARCH}" \
    &&   echo "  - papi" \
    &&   echo "  - mpich" \
    &&   echo "  - upcxx cflags=\"-fPIC\" cxxflags=\"-fPIC\"" \
    &&   echo "  - gotcha" \
    &&   echo "  - dyninst" \
    &&   echo "  - caliper~papi" \
    &&   echo "  - gperftools" \
    &&   echo "  - python" \
    &&   echo "  - py-scikit-build" \
    &&   echo "  - py-pip" \
    &&   echo "  concretization: together" \
    &&   echo "  config:" \
    &&   echo "    install_tree: /opt/software" \
    &&   echo "  view: /opt/view") > /opt/timemory/spack.yaml

# Install the software, remove unecessary deps
RUN cd /opt/timemory && \
    apt-get update && apt-get install -y wget gpg software-properties-common && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | apt-key add - && \
    DISTRIB_CODENAME=$(cat /etc/lsb-release | grep DISTRIB_CODENAME | awk -F '=' '{print $NF}') && \
    apt-add-repository "deb https://apt.kitware.com/ubuntu/ ${DISTRIB_CODENAME} main" && \
    apt-get update && apt-get install -y cmake build-essential ninja-build && \
    wget --no-check-certificate https://oaciss.nic.uoregon.edu/e4s/e4s.pub && \
    spack gpg trust e4s.pub && \
    spack mirror add E4S https://cache.e4s.io/e4s && \
    rm e4s.pub && \
    spack external find && \
    spack --env . install && spack gc -y

# Modifications to the environment that are necessary to run
RUN cd /opt/timemory && \
    spack env activate --sh -d . >> /etc/profile.d/z10_spack_environment.sh

# RUN export PATH=/opt/view/bin:$PATH && pip install numpy cython pillow matplotlib mpi4py

# Bare OS image to run the installed executables
FROM spack/ubuntu-bionic:latest as stager

COPY --from=builder /opt/timemory /opt/timemory
COPY --from=builder /opt/software /opt/software
COPY --from=builder /opt/view /opt/view
COPY --from=builder /etc/profile.d/z10_spack_environment.sh /etc/profile.d/z10_spack_environment.sh

ARG GCC_VERSION=8
ARG CLANG_VERSION=9
ARG ENABLE_DISPLAY=0

WORKDIR /tmp

# package-manager configuration
COPY ./apt.sh /tmp/apt.sh
RUN ./apt.sh && rm -rf /tmp/*

RUN echo 'export PS1="\[$(tput bold)\]\[$(tput setaf 1)\][timemory]\[$(tput setaf 2)\] \u\[$(tput sgr0)\]:\w $ \[$(tput sgr0)\]"' >> ~/.bashrc && \
    echo '. /opt/spack/share/spack/setup-env.sh' >> ~/.bashrc && \
    echo 'spack load cuda' >> ~/.bashrc && \
    apt-get update && \
    apt-get install -y python3.6-dev && \
    source /opt/spack/share/spack/setup-env.sh && \
    source /etc/profile.d/z10_spack_environment.sh && \
    spack load -r py-pip && \
    pip install numpy cython pillow matplotlib mpi4py && \
    echo 'export CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}:/opt/view/targets/x86_64-linux' >> ~/.bashrc && \
    apt-get -y autoclean && \
    rm -rf /var/lib/apt/lists/* && \
    update-alternatives --install /usr/bin/python python /usr/bin/python2 10 && \
    update-alternatives --install /usr/bin/python python /usr/bin/python3 20
    
FROM scratch

COPY --from=stager / /

ENV HOME /root
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US
ENV LC_ALL C
ENV SHELL /bin/bash
ENV BASH_ENV /etc/bash.bashrc
ENV DEBIAN_FRONTEND noninteractive

ARG REQUIRE_CUDA_VERSION=10.0
ENV CUDA_HOME "/opt/view"
ENV NVIDIA_REQUIRE_CUDA "cuda>=${REQUIRE_CUDA_VERSION}"
ENV NVIDIA_VISIBLE_DEVICES "all"
ENV NVIDIA_DRIVER_CAPABILITIES "compute,utility"

ENV CC /usr/bin/cc
ENV CXX /usr/bin/c++
ENV CUDACXX /opt/view/bin/nvcc

COPY ./etc/profile.d/*.sh /etc/profile.d/
COPY ./etc/bash.bashrc /etc/
COPY ./etc/compute-dir-size.py /etc/

LABEL "app"="timemory"
LABEL "mpi"="mpich"

USER root
WORKDIR /home
SHELL [ "/bin/bash", "--login", "-c" ]

# ENTRYPOINT [ "/runtime-entrypoint.sh" ]
ENTRYPOINT ["/bin/bash", "--rcfile", "/etc/profile", "-l"]
