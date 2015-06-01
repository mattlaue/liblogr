# Introduction #

**logr** is a C library that provides log file formatting and rotation in a similar fashion to  java's [java.util.logging package](http://docs.oracle.com/javase/6/docs/api/java/util/logging/package-summary.html) or [Python's logging module](http://docs.python.org/library/logging.html).

# Getting Started #

To build from the "git clone"d source tree, make sure you have "autoconf" and "libtool" installed.  For example, on Ubuntu:
<pre>
sudo apt-get install autoconf libtool<br>
</pre>
Then build it:
<pre>
./autogen.sh<br>
make<br>
</pre>
Then install it:
<pre>
sudo make install<br>
</pre>
When configured as above, with the default, it is installed into /usr/local.

## Examples ##
Examples for logging with various formats, etc. are in the **examples** directory.