#!/bin/perl 
use Getopt::Std;
#############################################################
#  creates the Wiki page for supported cards.  
#  MUST EDIT THE pageName and VERSION variables for each release
#
#############################################################

my $pageName = "SupportedCardList0181";
my $VERSION = "0.181";

#############################################################
#--- DO NOT EDIT BEYOND THIS LINE UNLESS YOU WANT TO CHANGE THE FORMAT!#
#############################################################

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

print OUTFILE<<WIKI;
#summary Supported Card List for $VERSION

= Introduction =

Supported Card list for $VERSION, includes Borderline cards

= Details =
$summaryMessage

WIKI

print OUTFILE $headerRow . "\n";;

foreach my $key ( @keys )
{
	my $cardCount = $#{$data->{$key}} + 1;
	print "Processing $key ($cardCount cards)\n";

	print OUTFILE "==$key";
	print OUTFILE " ($cardCount cards)";
	print OUTFILE "  _Back to [$pageName#Details Top]_" if ( $key ne "A");
	print OUTFILE "==\n";

	print OUTFILE map "  # $_\n", @{$data->{$key}};
}
close OUTFILE;
print "$totalCardCount Processed\n";

sub getHeader
{
	my $listRef = shift;
	my @list = ();
	my $header;
foreach my $key ( @keys )
{
	my $cardCount = $#{$data->{$key}} + 1;
	$header = "[$pageName#${key}_(${cardCount}_cards)_Back_to $key]";
	push @list, $header;
}
	shift( @list ); # remove the link to the first set of cards
	my $listItems =  join " ", @list;

	return "\n$listItems\n";

}
