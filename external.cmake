include(ExternalProject)

ExternalProject_Add(libzmq
    PREFIX ${EXTERNAL_DIR}
    SOURCE_DIR ${EXTERNAL_DIR}/libzmq
    INSTALL_DIR ${USR_DIR}
    GIT_REPOSITORY http://github.com/zeromq/libzmq.git
    GIT_TAG v4.3.5
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_TEST=OFF
        -DWITH_DOCS=OFF
        -DWITH_TLS=OFF
        -DZMQ_BUILD_TESTS=OFF
        -DBUILD_SHARED=OFF
        -DBUILD_STATIC=ON
        -DCMAKE_INSTALL_PREFIX:PATH=${USR_DIR}
)

ExternalProject_Add(zmqpp
    PREFIX ${EXTERNAL_DIR}
    SOURCE_DIR ${EXTERNAL_DIR}/zmqpp
    INSTALL_DIR ${USR_DIR}
    GIT_REPOSITORY http://github.com/zeromq/zmqpp.git
    GIT_TAG ba4230d
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DIS_TRAVIS_CI_BUILD=OFF
        -DZMQPP_BUILD_CLIENT=OFF
        -DZMQPP_BUILD_EXAMPLES=OFF
        -DZEROMQ_INCLUDE_DIR=${USR_DIR}/include
        -DZEROMQ_LIBRARY_STATIC=${USR_DIR}/lib/libzmq-static.a
        -DZMQPP_BUILD_SHARED=OFF
        -DZMQPP_BUILD_STATIC=ON
        -DZMQPP_LIBZMQ_CMAKE=ON
        -DCMAKE_INSTALL_PREFIX:PATH=${USR_DIR}
)

ExternalProject_Add(snappy
    PREFIX ${EXTERNAL_DIR}
    SOURCE_DIR ${EXTERNAL_DIR}/snappy
    INSTALL_DIR ${USR_DIR}
    GIT_REPOSITORY http://github.com/google/snappy.git
    GIT_TAG 27f34a5
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DSNAPPY_BUILD_BENCHMARKS=OFF
        -DSNAPPY_BUILD_TESTS=OFF
        -DCMAKE_INSTALL_PREFIX:PATH=${USR_DIR}
)

add_dependencies(zmqpp libzmq)
