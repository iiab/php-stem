## php-stem

php-stem requires three files:

* A stem.so shared object compiled from c
* A stem.ini definition file that is the same for all versions
* A symlink which is created in the ansible install task
* /etc/php/{{ php_version }}/fpm/conf.d/20-stem.ini -> /etc/php/{{ php_version }}/mods-available/stem.ini

### Building the so file

The stem.so file depends on arch and php version.

* `apt install php-dev`
* `cd to src`
* `phpize`
* `./configure`
* `make`
* `make test` (might fail ...?)


### Installing/Enabling the Module
Note: These steps tested on Ubuntu 22.04 with PHP 8.1 and php-fpm.  Paths on other distros may vary.

* ... while still in `src` directory:
* Install module
  * `sudo make install`
  * This will copy the .so file into `/usr/lib/php/yyyymmdd/' where yyyymmdd is the date computed by phpize
* Create .ini
  * `sudo cp stem.ini /etc/php/8.1/mods-available/`
* Enable module for php-fpm
  * `sudo phpenmod stem`
* Reload php-fpm
  * `sudo service php8.1-fpm reload`



### Source

The source included here can be obtained by

* apt install subversion
* svn co http://svn.php.net/repository/pecl/stem
* cd stem/trunk/
* (using version 1.5, not 1.5.1)
