#!/bin/perl 

use Getopt::Std;

# declare the perl command line flags/options we want to allow
my %options=();
getopts("i:t:", \%options);

my $currentIndex = 0;
my %data = ();

my $outputFile = "cardList.html";
if ( $options{t} =~ /todo/i)
{
	$outputFile = "todoList.html";
}

print "Processing $options{i}\n";
open INFILE, "<$options{i}" || die "$options{i} can't open for reading. $!\n";
while ( <INFILE> )
{
	s///;
	s/\n//;
	my $line = $_;
	my $currentIndex = uc substr($line,0,1);
	if ($line =~ /^\d/ )
	{
		push @{$data->{"0-9"}}, $line;
	}

	else
	{
		push @{$data->{$currentIndex}}, $line;
	}
}
close INFILE;

my @keys = sort keys %$data;
foreach (@keys)
{
	my $cardCount = scalar @{$data->{$_}};
#	print $_ . " => $cardCount \n";
	$totalCardCount += $cardCount;
}
my $summaryMessage = "There are a total of $totalCardCount cards supported in this release.";
my $headerRow = &getHeader( \@keys );

open OUTFILE, ">$outputFile" || die "$0: Can't write to $outputFile. $!\n";

#print index keys of cards

# print the miki card count information

print OUTFILE<<MIKI;
$summaryMessage

MIKI


print OUTFILE $headerRow . "\n";;

foreach my $key ( @keys )
{
	my $cardCount = $#{$data->{$key}} + 1;
	print "Processing $key ($cardCount cards)\n";

	print OUTFILE "<h2 style=\"width: 100%\"><span id=\"$key\">";
	print OUTFILE "$key" ;
	print OUTFILE " ($cardCount)" if ($cardCount > 10 );
	print OUTFILE "</span> <span style=\"font-size:small; float: right;\">Back to [[#Top]]</span></h2>\n";

	print OUTFILE "<ul id=\"index_$key\">\n";
	print OUTFILE map "\t<li>$_</li>\n", @{$data->{$key}};
	print OUTFILE "</ul>\n\n";
}
close OUTFILE;
print "$totalCardCount Processed\n";


sub getHeader
{
	my $listRef = shift;
	my @list =  map "<li style=\"display:inline; list-style-type: none; right-padding: 5px\" >[[#$_]]</li>", @{$listRef};

	shift( @list ); # remove the link to the first set of cards
	my $listItems =  join "\n", @list;

	return "__NOTOC__ \n<ul id=\"Top\">\n$listItems\n</ul>\n";

}
