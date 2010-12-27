#!/bin/perl 

my $currentIndex = 0;
my %data;
while ( <> )
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

my @keys = sort keys %$data;
foreach (@keys)
{
	my $cardCount = scalar @{$data->{$_}};
	print $_ . " => $cardCount \n";
	$totalCardCount += $cardCount;
}

my $headerRow = &getHeader( \@keys );

open OUTFILE, ">cardList.txt";

#print index keys of cards

# print the miki card count information

print OUTFILE<<MIKI;
There are a total of $totalCardCount cards supported in this release.

MIKI


print OUTFILE $headerRow . "\n";;

foreach my $key ( @keys )
{
	my $cardCount = $#{$data->{$key}} + 1;
	print OUTFILE "<h2 style=\"width: 100%\"><span id=\"$key\">";
	print OUTFILE "$key" ;
	print OUTFILE " ($cardCount)" if ($cardCount > 10 );
	print OUTFILE "</span> <span style=\"font-size:small; float: right;\">Back to [[#Top]]</span></h2>\n";

	print OUTFILE "<ul id=\"index_$key\">\n";
	print OUTFILE map "\t<li>$_</li>\n", @{$data->{$key}};
	print OUTFILE "</ul>\n\n";
	close OUTFILE;

}


sub getHeader
{
	my $listRef = shift;
	my @list =  map "<li style=\"display:inline; list-style-type: none; right-padding: 5px\" >[[#$_]]</li>", @{$listRef};

	shift( @list ); # remove the link to the first set of cards
	my $listItems =  join "\n", @list;

	return "__NOTOC__ \n<ul id=\"Top\">\n$listItems\n</ul>\n";

}
