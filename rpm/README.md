# Building RPMs

1. Install `docker` and `docker-compose`
2. `cd rpm`
3. Build the docker image: `docker-compose build rpm`
3. Build the rpms: `docker-compose run rpm` (Can take over half an hour)
4. Now you can run `dnf install RPMS/x84_64/darling*.rpm`
5. `setsebool -P mmap_low_allowed 1` to allow darling low level access and run

## Building on other operating systems

By default, it will build for Fedora 30. To use a different OS, simply

    RPM_OS=fedora:31 docker-compose build rpm

## Future improvements

- Everything is based off of dnf. Supporting zypper and yum will reach others
- Because of the way the submodules are handled, this isn't quite ready for official releasing but this can be solved using [%autosetup in the %prep to checkout the submodules.](https://fedoraproject.org/wiki/Packaging:SourceURL#Git_Submodules)