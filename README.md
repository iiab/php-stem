## php-stem

php-stem requires three files:

* A stem.so shared object compiled from c
* A stem.ini definition file that is the same for all versions
* A symlink which is created in the ansible install task
* /etc/php/{{ php_version }}/fpm/conf.d/20-stem.ini -> /etc/php/{{ php_version }}/mods-available/stem.ini

### Building the so file

The stem.so file depends on arch and php version.

* Apt install php-dev
* cd to src
* phpize
* ./configure
* make
* make test

See the README in the src directory

### Deployment

The stem.ini file goes in /etc/php/N.M/mods-available/ where N and M are the major and minor versions of php

The stem.so file goes in /usr/lib/php/yyyymmdd/ where yyyymmdd is the date computed by phpize

### Source

The source included here can be obtained by

* apt install subversion
* svn co http://svn.php.net/repository/pecl/stem
* cd stem/trunk/
* (using version 1.5, not 1.5.1)
