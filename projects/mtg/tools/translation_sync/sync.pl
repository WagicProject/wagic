#!/usr/bin/perl

#This script synchronizes a translation of _cards.dat with the equivalent english _cards.dat
# it keeps everything from the english file, except the text line that is taken from the translated file
# usage: translate.pl FR

use strict;
use warnings;
use Cwd;
use DirHandle;
use Data::Dumper;


# Subroutines


my $parseCards = sub {
    my $inFile = shift;
	return {} unless (-f $inFile);
	my @cards;
	my %card;
	open (MYFILE, $inFile);
	while (<MYFILE>) {
		chomp;
		my $line = $_;
		next if $line eq "[card]";
		if ($line =~ /\[\/card\]/) {
			my %copy = %card;
			%card =();
			push @cards, \%copy;
		}
		if ($line =~ /=/){
			my ($type, $content) = split(/=/, $line);
			$card{$type} = $content;
		}
		
	}
	close (MYFILE);
 	

	my %cards = map { $_->{id} => $_ } @cards;
	return \%cards;
};

my $doOutput = sub {
	my ($outFile, $data, $translation) = @_;
	open (OUTFILE, '>' . $outFile);



	
	while ( my ($key, $value) = each(%$data) ) {
		if (my $trans = $translation->{$key}){
			$value->{text} = $trans->{text} if $trans->{text};
		}
		print OUTFILE "[card]\n";
		while ( my ($k, $v) = each(%$value) ) {
			print OUTFILE $k ."=" . $v . "\n";
		}
		print OUTFILE "[/card]\n";
		
    }	
	
	close (OUTFILE); 
};

my $output = sub {


};

my $lang = $ARGV[0] || "FR";
my $data_folder = "../../bin/Res/sets";
my $input_translation = "$lang/sets";
my $output_folder = "output";


my $dh = DirHandle->new($input_translation) or die "No such directory: $input_translation";
print "looking for translated sets in $input_translation\n";

$dh = DirHandle->new($data_folder) or die "No such directory: $data_folder";

while(defined(my $val = $dh->read)){
	next if "." eq $val;
	next if ".." eq $val;
	if (-d $data_folder."/".$val){ 
		if (-f $data_folder. "/" . $val ."/" . "_cards.dat"){
				print "processing $val \n";
				my $curdir = $input_translation."/".$val;
				my $cards = $parseCards->($data_folder. "/" . $val ."/" . "_cards.dat");
				my $transl = $parseCards->($curdir."/_cards.dat");
				my $out = $output_folder ."/" . $val;
				mkdir ($out);
				$doOutput->($out . "/_cards.dat", $cards,$transl);
				if (%$transl){
					print "-->OK\n";
				}else{
					print "--> No translation was found for $val. The original file was used\n";
				}				
		}
	}
}
print "done! resulting _cards.dat files put in Folder:$output_folder\n";
$dh->close();