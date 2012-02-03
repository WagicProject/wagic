#!/usr/bin/perl -w
# This is a very rudimentary tool to check that the cards in mtg.txt, borderline.txt, etc have valid abilities.  Must update @abilities whenever a new ability is added to the game.  

#Usage: validateAbilities.pl < [path to mtg.txt] 

my @abilities = (
    "trample", "forestwalk", "islandwalk", "mountainwalk", "swampwalk", "plainswalk", "flying", "first strike", "double strike", "fear", "flash", "haste", "lifelink", "reach", "shroud", "vigilance", "defender", "banding", "protection from green", "protection from blue", "protection from red", "protection from black", "protection from white", "unblockable", "wither", "persist", "retrace", "exalted", "nofizzle", "shadow", "reachshadow", "foresthome", "islandhome", "mountainhome", "swamphome", "plainshome", "cloud", "cantattack", "mustattack", "cantblock", "doesnotuntap", "opponentshroud", "indestructible", "intimidate", "deathtouch", "horsemanship", "cantregen", "oneblocker", "infect", "poisontoxic", "poisontwotoxic", "poisonthreetoxic", "phantom", "wilting", "vigor", "changeling", "absorb", "treason", "unearth", "cantlose", "cantlifelose", "cantmilllose", "snowlandwalk", "nonbasiclandwalk", "strong", "storm", "phasing", "split second", "weak", "affinityartifacts", "affinityplains", "affinityforests", "affinityislands", "affinitymountains", "affinityswamps", "affinitygreencreatures", "cantwin", "nomaxhand", "leyline", "playershroud", "controllershroud", "sunburst", "flanking", "exiledeath", "legendarylandwalk", "desertlandwalk", "snowforestlandwalk", "snowplainslandwalk", "snowmountainlandwalk", "snowislandlandwalk", "snowswamplandwalk", "canattack", "hydra", "undying", "poisonshroud" );

my %abilityMap = ();
chomp @abilities;

foreach (@abilities)
{
 s/"//g;
  $abilityMap{$_} = 1;
}

my @list = grep /abilities=/, <>;

chomp (@list);
foreach my $item (sort @list)
{
	my ($tag, $abilitiesStr) = split /=/, $item;
	my @abilityList = split ",", $abilitiesStr;
	foreach my $ability (@abilityList)
	{
		$ability =~ s/\s+$//g;
		$ability =~ s/^\s+//g;
		$ability = lc($ability);
		if (!defined ($abilityMap{$ability} ))
		{
			print STDERR "Ability not found! : $ability\n";
		}
	}

}


#print map "[$_]\n", keys %abilityMap;
