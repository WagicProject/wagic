#!/bin/bash

####################################################
#  adjust the values for mtg.txt and borderline.txt
#  if your "Res" directory isn't in the default place
####################################################

####################################################
#  Purpose:  build an HTML list of all the supported 
#	cards in Wagic for use in the Miki.
#	http://wololo.net/.
####################################################
# create a summary of the borderline and fully supported cards
grep '^name=' ../../bin/Res/sets/primitives/mtg.txt ../../bin/Res/sets/primitives/borderline.txt|sed s/name=// | sed s/.*txt:// |sort -iu > supported_cards.txt

# create a summary of the TODO card sets
grep '^name=' ../../bin/Res/sets/primitives/unsupported.txt |sed s/name=// > todo_cards.txt

perl createHTMLList_SupportedCards.pl -i supported_cards.txt 
perl createHTMLList_SupportedCards.pl -i todo_cards.txt -t todo
