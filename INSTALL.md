# Build and installation instructions

## Build dependencies

 - cmake
 - cmake-extras
 - glib2
 - glib2-devel
 - gobject-introspection
 - vala
 - gi-docgen
 - dbus-test-runner - **For testing**
 - xorg-server-xvfb - **For testing**
 - gcovr - **For coverage**
 - lcov - **For coverage**

**The install prefix defaults to `/usr`, change it with `-DCMAKE_INSTALL_PREFIX=/some/path`**

## For end-users and packagers

```
cd libayatana-appindicator
mkdir build
cd build
cmake ..
make
sudo make install
```

## For testers - tests only

```
cd libayatana-appindicator
mkdir build
cd build
cmake .. -DENABLE_WERROR=ON -DENABLE_TESTS=ON
make
make test
```

## For testers - both tests and code coverage

```
cd libayatana-appindicator
mkdir build
cd build
cmake .. -DENABLE_WERROR=ON -DENABLE_COVERAGE=ON
make
make coverage
```
