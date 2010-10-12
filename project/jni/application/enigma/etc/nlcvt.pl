#!/usr/bin/perl -w
# nlcvt - convert newline notations
# Tom Christiansen, 9 March 1999

#   "The most brilliant decision in all of Unix was 
#    the choice of a *single* character for the 
#    newline sequence.      --Mike O'Dell, only half jokingly

use strict;

END {
    close STDOUT            || die "$0: can't close stdout: $!\n";
    $? = 1 if $? == 255;    # from die
} 

my(
    $src,		# input format style
    $dst,		# output format style
    %format,		# table of conversion
    $errors, 		# file input errors
);

$errors = 0;

%format = (

    # the good...

    "unix"		=> "\cJ",	# CANON
    "plan9"		=> "\cJ",
    "inferno"		=> "\cJ",
    "linux"		=> "\cJ",	# some people don't get it
    "bsd"		=> "\cJ",	# some people don't get it
    "be"		=> "\cJ",
    "beos"		=> "\cJ",

    # the not so good, but still ok...

    "mac"		=> "\cM", 	# CANON
    "apple"		=> "\cM",
    "macintosh"		=> "\cM", 

    # and the really unbelievably idiotic...

    "cpm"		=> "\cM\cJ",	# CANON
    "cp/m"		=> "\cM\cJ",	# could be in first arg
    "dos"		=> "\cM\cJ",
    "windows"		=> "\cM\cJ",
    "microsoft"		=> "\cM\cJ",
    "nt"		=> "\cM\cJ",
    "win"		=> "\cM\cJ",

);

sub usage {    
    warn "$0: @_\n" if @_;
    my @names = sort { 
			$format{$a} cmp $format{$b} 
				     ||
				$a cmp $b 
    } keys %format;
    my $fmts = "@names";
    print STDERR "usage: $0 src2dst [file ...]\n";
    format STDERR = 
    where src and dst are both one of:
~~      ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	$fmts
.
    write STDERR;
    exit(1);
}

($src, $dst) = ($0 =~ /(\w+)2(\w+)/);

usage("insufficient args") unless @ARGV || ($src && $dst);

if (@ARGV && $ARGV[0] =~ /(\w+)2(\w+)/) {
    ($src, $dst) = ($1, $2);
    shift @ARGV;
} 

usage("no conversion specified") unless $src && $dst;

usage("unknown input format: $src")  unless $/ = $format{lc $src};
usage("unknown output format: $dst") unless $\ = $format{lc $dst};

binmode(STDOUT);

unshift @ARGV, '-' unless @ARGV;

for my $infile (@ARGV) {
    unless (open(INPUT, $infile)) {
	warn "$0: cannot open $infile: $!\n";
	$errors++;
	next;
    } 


    binmode(INPUT);

    unless (-T INPUT) {
	warn "$0: WARNING: $infile appears to be a binary file.\n";
	$errors++;
    } 

    while (<INPUT>) {
	unless (chomp) {
	    $errors++;
	    warn "$0: WARNING: last line of $infile truncated, correcting\n";
	} 
	print;
    } 

    unless (close INPUT) {
	warn "$0: cannot close $infile: $!\n";
	$errors++;
	next;
    } 
} 

exit ($errors != 0);

__END__

=head1 NAME

nlcvt - convert foreign line terminators

=head1 SYNOPSIS

B<nlcvt> I<src>2I<dst> [I<file> ...]

B<unix2mac> [I<file> ...]

B<unix2cpm> [I<file> ...]

B<cpm2unix> [I<file> ...]

B<cpm2mac> [I<file> ...]

B<mac2unix> [I<file> ...]

B<mac2cpm> [I<file> ...]

=head1 DESCRIPTION

Mike O'Dell said, only half-jokingly, that "the most brilliant decision
in all of Unix was the choice of a I<single> character for the newline
sequence."  But legacy systems live on past their days, and these programs
can help that.  Note, however, that if you've downloaded a binary file in
"text" mode rather than "binary", your mileage may vary.

The B<nlcvt> program, or any of its many aliases, is a filter to convert
from one system's notion of proper line terminators to that of another.
This usually happens because you've downloaded or otherwise directly
transferred a text file in so-called "binary" rather than "text" mode.

Unix format considers a lone Control-J to be the end of line.  Mac format
considers a lone Control-M to be the end of line.  The archaic CP/M
format considers a Control-M and a Control-J to be the end of line.

This program expects its first argument to be of the form I<src>2I<dst>,
where I<src> and I<dst> are both one of B<unix>, B<mac>, or B<cpm>.
(That's speaking canonically--many aliases for those systems exist: call
B<nlcvt> without arguments to see what names are accepted.)  The converted
data is written to the standard output.  B<nlcvt> does I<not> do 
destructive, in-place modification of its source files.  Do this
instead:

    cpm2unix < file.bad > file.good 
    mv file.good file.bad

This program can also be called by the name of the conversion itself.
Just create links to the B<nlcvt> program for each systems, and the
program use its own name to determine the conversion.  For example:

    #!/usr/bin/perl
    # make nlcvt links
    chomp($path = `which nlcvt`);
    @systems = qw(unix mac cpm);
    for $src (@systems) {
	for $dst (@systems) {
	    next if $src eq $dst;
	    ln($path, "${src}2$dst") || die $!;
	} 
    } 

=head1 DIAGNOSTICS

Any of the following diagnostics cause B<nlcvt>
to exit non-zero.

=over

=item C<insufficient args>

You called the program by its canonical name, 
and supplied no other arguments.
You must supply a conversion argument.

=item C<no conversion specified>

Neither the name of the program nor its
first argument were of the form I<src>2I<dst>.

=item C<unknown input format: %s>

The specified input format, C<%s>, was unknown.
Call B<nlcvt> without arguments for a list of
valid conversion formats.

=item C<unknown output format: %s>

The specified output format, C<%s>, was unknown.
Call B<nlcvt> without arguments for a list of
valid conversion formats.

=item C<cannot open %s: %m>

The input file C<%s> could not be opened for the reason
listed in C<%m>.

=item C<cannot close %s: %m>

The input file C<%s> could not be close for the reason
listed in C<%m>.  This error is rare.

=item C<can't close stdout: %m>

The filter could not finish writing to its standard output for the
reason listed in C<%m>.  This could be caused by a full or temporarily
unreachable file system.

=item C<WARNING: last line of %s truncated, correcting>

Text files contain zero or more variable-length, newline-terminated
records.  Occasionally, the final record terminator is missing,
perhaps due to an incomplete transfer, perhaps due to an aberrant
I<emacs> user.  A newline sequence appropriate to the destination
system is appended.  This would be a valid use of a I<unix2unix>
conversion.  And no, you can't call it as B<emacs2vi>.

=item C<WARNING: %s appears to be a binary file>

Perl's C<-T> operator did not think the input file was a text file.
The conversion is still performed, but is of dubious value.  If
the file really was binary, the resulting output may be mangled.
Garbage in, garbage out.

=back

=head1 AUTHOR

Tom Christiansen, I<tchrist@perl.com>.

=head1 COPYRIGHT

This program is copyright (c) 1999 by Tom Christiansen.

This program is free and open software. You may use, copy, modify,
distribute, and sell this program (and any modified variants) in any
way you wish, provided you do not restrict others from doing the same.
