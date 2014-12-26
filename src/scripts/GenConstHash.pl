#!/usr/bin/perl -w
#----------------------------------------------------------------------------
#"THE BEER-WARE LICENSE" (Revision 42):
#<chris@chrissavoie.com> wrote this file. As long as you retain this notice you
#can do whatever you want with this stuff. If we meet some day, and you think
#this stuff is worth it, you can buy me a beer in return Chris Savoie
#----------------------------------------------------------------------------
#
# Generate a compile-time hash macro into the given file
# with the given range of characters

use File::Basename;

$target = "hashmacro.h";
$intermediate = "";
$usetest = 0;
$test = "";
$hashfn = "";
$hashdepth = "";
$root = "HASH";
$limit = 32;
$function = "";
$verbose = 0;

sub printusage()
{
	print "\n";
	print "Usage: genhashmacro [args]\n";
	print " If the target file already exists it will look for the existing macro name and reuse it.\n";
	print "  -l [number]    The depth of recursion to use, default $limit\n";
	print "  -r [name]      Root macro name, default $root\n";
	print "  -t [file]      Target file to write to, default $target\n";
	print "  -int [dir]		Intermediate directory for test file\n";
	print "  -v             Verbose output\n";
	print "  -h             Print help\n";
}

sub parsecommandline()
{
	for ($i = 0; $i < @ARGV; $i++)
	{
		my $handled = 0;
		
		$arg = $ARGV[$i];
		$arg1 = "";
		if ($i < @ARGV - 1)
		{
			$arg1 = $ARGV[$i+1];
		}
		
		if ($arg =~ /-[hH]/)
		{
			printusage();
			exit;
		}
		elsif ($arg =~ /-[lL]/)
		{
			$limit = $arg1;
			$i++;
			if ($verbose) { print "Using limit: $limit\n"; }
		}
		elsif ($arg =~ /-[tT]/)
		{
			$target = $arg1;
			$i++;
			if ($verbose) { print "Output file: $target\n"; }
		}
		elsif ($arg =~ /-[rR]/)
		{
			$root = $arg1;
			$i++;
			if ($verbose) { print "Root macro name: $root\n"; }
		}
		elsif ($arg =~ /-int/)
		{
			$intermediate = $arg1;
			$usetest = 1;
			$i++;
			if ($verbose) { print "Using Intermediate file\n"; }
		}
		elsif ($arg =~ /-[vV]/)
		{
		    $verbose = 1;
			print "Verbose: on\n";
		}
		else
		{
			print "Invalid argument: $arg\n";
			printusage();
			exit;
		}
	}
}

sub printfile()
{
	#convert the filename to a header guard
	my($filename, $directories, $suffix) = fileparse($target);
	$define = uc "$filename"."$suffix";
	$define =~ s/\./_/g;
	my $hashconstants = "$root\_CONSTANTS";
	my $hashmults = "$root\_MULTS";
	
    open F,">$target" or die("Cannot create output file \"$target\"\n");
    
	print F "
#ifndef $define
#define $define

// the depth of the hashing
#define $hashdepth $limit

// randomly generated constants.  The bottom half has to be FFFF or
// else the entire hash loses some strength
static const size_t $hashconstants\[$hashdepth+1\] =
{";
    my $linelimit = sqrt($limit);
    srand(132465);
    for (my $i = 0; $i <= $limit; $i++)
    {
        if ($i % $linelimit == 0)
        {
            print F "\n    ";
        }
        printf F "0x%08X, ", int(rand(0xffffffff));
    }

print F "
};

// multiplication constants, this allows an abstract use
// of the string length
static const size_t $hashmults\[$hashdepth+1\] =
{";

    for (my $i = 0; $i <= $limit; $i++)
    {
        if ($i % $linelimit == 0)
        {
            print F "\n    ";
        }
        printf F "% 3d, ", 33 + $i;
    }
    
print F "
};

";
	my $formatter = "%02d";
	if ($limit > 100)
	{
		$formatter = "%03d";
	}
	if ($limit > 1000)
	{
		$formatter = "%04d";
	}

    my $string = "string";
    my $value = "value";
    my $number = 0;
	my $name = "$root\_RECURSE\_$formatter";
    while ($number < $limit)
    {
        my $mult = int(rand(0xffffffff));
        printf F "#define $name($string, $value) $hashfn((*($string+1) == 0 ? $hashconstants\[%d\] : $hashmults\[%d\] * $name($string+1, *($string+1))), $value)\n", $number, $number, $number, $number+1;
        $number++;
    }

    printf F "#define $name($string, $value) $hashconstants\[%d\]\n", $limit, $limit;
    print F "
// The following is the function used for hashing
// Do NOT use NEXTHASH more than once, it will cause
// N-Squared expansion and make compilation very slow
// If not impossible
$function

// finally the macro used to generate the hash
";
	printf F "#define $root($string) $name($string, *$string)\n", 0;
	print F "
#endif // $define
";
	
	close F;

	# create a test file to check for date to see if it has changed
   # open T,">$test" or die("Cannot create output file \"$test\"\n");
   # close T;
}

sub main()
{
	parsecommandline();
	$test = "$intermediate$target.test";
    $hashfn = $root."_FUNCTION";
    $hashdepth = $root."_DEPTH";
	$function = "#define $hashfn(next, value) (value + 33) + ((11 * value * (value << 3)) ^ (next))";

	if (-e $target)
	{
		if ($usetest and -e $test)
		{
			@source = stat $target;
			@test = stat $test;
			
			# 9 is modification time
			if ($source[9] <= $test[9])
			{
				if ($verbose) { print "$target - Up to date"; }
				exit;
			}
		}
		else
		{
		    if ($verbose) { print "Checking old file for hash macro\n"; }
			# if the file already exists check for an existing hash macro before
			# overwriting it
			open READ, "<$target" or die "Error: Failed to open file $target";

			while (<READ>)
			{
				chomp;
				if ($_ =~ /#define $hashfn/)
				{
					$function = $_;
					if ($verbose) { print "Reusing hash function $_\n"; }
					goto DONEREADING;
				}
			}
			DONEREADING:

			close READ;
		}
	}
	else
	{
		if ($verbose) { print "Generating hash file.\n"; }
	}

    printfile();
}
main();
