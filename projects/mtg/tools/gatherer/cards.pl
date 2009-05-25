#!/usr/bin/perl

my $data_file="IN.txt";
open(DAT, $data_file) || die("Could not open file!");
my @raw_data=<DAT>;
close(DAT);

my $state = 0;
my $textLines = 0;
my $count = 0;
foreach my $line (@raw_data)
{
    my $id;
    my $name;
    $count++;

    if ($line =~ m/(\s*)<a id="ctl00_ctl00_ctl00_MainContent_SubContent_SubContent_ctl00_cardEntries_ctl(.*)_cardLink" (.+) href="..\/Card\/Details.aspx\?multiverseid\=(.+)">(.+)<\/a>/g ) {
        $id = $4;
        $name = $5;
        if ($state) {
            print "[/card]\n";
        }
        $state = 1;

        print "[card]\n";
        print "id=" . $id . "\n";
        print "name=" . $name . "\n";
    }else {
        next unless $state;
        $state++;
        $line =~ s/^\s*(.*?)\s*$/$1/;
        if ($state == 9) {
            my @manas = split(//,$line);
            print "mana=";
            foreach my $mana (@manas) {
                print "{$mana}";
            }
            print "\n";
        }
        if ($state == 17) {
            if ($line =~ m/(.*)\s\s— (.+)/g) {
                my $type =$1;
                my $subtype = $2;
                print "type=" . $type . "\n";
                print "subtype=" . $subtype . "\n";
            } else {
                if ($line =~ m/(.+)/g) {
                    my $type =$1;
                    print "type=" . $type . "\n";
                }
            }

        }
        if ($state == 25) {
            if ($line =~ m/\((.+)\/(.+)\)/g) {
                my $p =$1;
                my $t =$2;
                print "power=" . $p. "\n";
                print "toughness=" . $t. "\n";
            }
        }
        if ($state == 33) {
            $line =~ s/<br\s\/>/ /;
            if ($line) {
                
                unless ($textLines) {
                    print "text=";
                }
                if ($line eq "</td>") {
                    print "\n";
                    $textLines = 0;
                    $state++;
                }else {
                    print $line;
                    $textLines++;
                    $state--;
                }
            }
        }
        if ($state == 41) {
            my $rarity = "C";
            if ($line =~ m/.+\sUncommon/g) {
                $rarity= "U";
            }
            elsif ($line =~ m/.+\sMythic Rare/g) {
                    $rarity= "M";
            }
            elsif ($line =~ m/.+\sRare/g) {
                    $rarity= "R";
            }
            print "rarity=" . $rarity . "\n";
        }

    }
}
print "[/card]\n";
