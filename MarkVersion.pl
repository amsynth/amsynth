#
# Script to automatically version files
# (C) 2005 Nick Dowell
#

my @cmd_locations = (
	"svnversion",
	"/usr/bin/svnversion",
	"/usr/local/bin/svnversion",
	"\"c:\\Program Files\\Subversion\\bin\\svnversion.exe\""
);


$numArgs = $#ARGV + 1;
if ($numArgs < 2)
{
	print "usage: MarkVersion.pl WorkingCopyPath SrcVersionFile [DstVersionFile]\n";
	exit;
}

my $cmd = 0;
for (@cmd_locations) {
	if (-1 != index `$_ --help 2>&1`, "usage:") {
		$cmd = $_; last;
	}
}
$cmd or die "could not locate svnversion";

$wc = $ARGV[0];
$src = $ARGV[1];
if ($numArgs > 2) { $dest = $ARGV[2]; } else { $dest = $src; }

print "svnversion          = $cmd\n";
print "WorkingCopyPath     = $wc\n";
print "SrcVersionFile      = $src\n";
print "DstVersionFile      = $dest\n";


# get version of local working copy
$wcrev = `$cmd $wc`;
chop $wcrev;

print "WorkingCopyRevision = $wcrev\n";

# get verision string, if specified in VERSION file
$version = "\$VERSION\$";
if (-e "$wc/VERSION")
{
	open FILE, "< $wc/VERSION" or die "could not open $wc/VERSION for input";
	$version = <FILE>;
	$version =~ s/\n//;
	close FILE;
}

print "Version             = $version\n";

open FILE, "< $src" or die "could not open $src for input : $!";
$data = join "", <FILE>;
close FILE;

$data =~ s/\$WCREV\$/$wcrev/g;
$data =~ s/\$VERSION\$/$version/g;

open FILE, "> $dest" or die "could not open $dest for output : $!";
print FILE $data;
close FILE;
