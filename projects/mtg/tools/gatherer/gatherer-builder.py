#!/usr/bin/python
#
# Requires BeautifulSoup verion 3, available from
# http://www.crummy.com/software/BeautifulSoup/
#
# Usage: python gatherer-builder.py <output file>
#
# Copyright 2006: Nathan Callahan
# Feel free to do what you want with this file, but give credit
# where it's due.


from BeautifulSoup import BeautifulSoup, BeautifulStoneSoup, Tag
import re
import codecs
import sys
import urllib
import os
import os.path
from mtgCommon import *

setinfo=sets['UZ']
stripReminderText = False
conffile = open(setinfo['dir'] + ".conf", 'w')

#FETCH_IMAGES = False
FETCH_IMAGES = True

url = "http://ww2.wizards.com/gatherer/Index.aspx?setfilter=%s\\&output=Spoiler" % setinfo['gathname']
gatherer = urllib.urlopen(url)
soup = BeautifulSoup(gatherer.read(), smartQuotesTo=None)

xml = BeautifulStoneSoup('<?xml version=\'1.0\' encoding=\'latin1\'?><!DOCTYPE ccg-setinfo SYSTEM "../gccg-set.dtd"><ccg-setinfo name="%s" dir="%s" abbrev="%s" game="Magic The Gathering"><cards></cards></ccg-setinfo>' % (setinfo['name'], setinfo['dir'], setinfo['abbrev']),selfClosingTags=['attr'])

rarity_re=re.compile(".*%s_(?P<rarity>.)\.gif.*" % setinfo['gathabbrev'])


def fetchImage(id, filename):
    if (not os.path.exists(setinfo['abbrev'] + "/" + filename)):
      for i in setinfo['gathdirs']:
        url="http://resources.wizards.com/Magic/Cards/%s/Card%s.jpg" % (i, id)
        print url
        try:
            pic = urllib.urlopen(url)
        except:
            pass
      if (not pic): # this is completely wrong, supposed to check if it's not found 
        raise IOError

      if (not os.path.exists(setinfo['abbrev'])):
        os.mkdir(setinfo['abbrev'])
      else:
        assert os.path.isdir(setinfo['abbrev'])
      f = open(setinfo['abbrev'] + "/" + filename, 'wb')
      f.write(pic.read())
      f.close()


    
for cardRow in soup.find(id="_gridResults").findAll('tr',onmouseover="this.style.backgroundColor='#F5DEB3';"):
    name = cardRow('b')[0].string
    name = name.replace('"','')
    name = name.replace(u'\xe2', 'a')
    name = name.replace(u'\xc6', 'AE')
    name = name.replace(u'\xe9', 'e')
    name = name.replace(u'\xe0', 'a')
    
    manaCost = replaceSymbols(cardRow('td')[1]('font')[0])
    manaCost = ''.join(manaCost.contents)
    print manaCost
    if manaCost == "&nbsp;":
        manaCost="";

    htmlText = cardRow('td')[3]
    htmlText = replaceSymbols(htmlText)
    text = cleanupHTMLText(htmlText, stripReminderText)
    text = text.replace(u'\xc6', 'AE')
    text = text.replace(u'\xa0', '')
    
    supertype, subtype = getCardTypes(cardRow)

    splitCard = split_re.match(text)
    if splitCard:
        text = splitCard.group('t1') + "    //    " + splitCard.group('t2')
        manaCost = manaCost + " // " + splitCard.group('mana2')
        supertype = supertype + " // " + splitCard.group('type2')
        
    power = cardRow('td')[4]('font')[0].string
    if power == "&nbsp;":
        power = None
    
    toughness = cardRow('td')[5]('font')[0].string
    if toughness == "&nbsp;":
        toughness = None
    
    colors = set()
    for c in manaCost:
        if c in symbolColors:
            colors.add(symbolColors[c].capitalize())
    color = ''.join(sorted([c+" " for c in colors])).rstrip()
    if not color:
        if (supertype.find("Artifact") != -1):
            color = "Artifact"
        elif (supertype.find("Land") != -1):
            color = "Land"
        else:
            ss = "%s is " % name
            start = text.find(ss) + len(ss)
            end = text.find('.',start)
            color = text[start:end].capitalize()
    
    printings = 1
    for printing in cardRow('td')[6].findAll(src=rarity_re):
	print name
        if name in basic_lands:
            rarity = 'L'
        else:
            rarity = rarity_re.match(str(printing)).group('rarity')

        card = Tag(xml, 'card')
        cards=xml('ccg-setinfo')[0]('cards')[0]

        cards.insert(len(cards),card)
        card=cards('card')[-1]

        card['name']=name

        if printings > 1:
            card['graphics']=name.translate(imagetrans)+str(printings)+".jpg"
        else:
            card['graphics']=name.translate(imagetrans)+".jpg"

	id = id_re.match(printing.parent['onclick']).group('id')
        if FETCH_IMAGES:
            fetchImage(id, id + ".jpg")





        
    	text = text.replace(u'\xe2', 'a')
        card['text']=text

        card.insert(0,Tag(xml,'attr'))
        card('attr')[0]['key']='rarity'
        card('attr')[0]['value']=rarity
        card.insert(1,Tag(xml,'attr'))
        card('attr')[1]['key']='color'
        card('attr')[1]['value']=color

        conffile.write("[card]")
#	conffile.write("\nimage=" + card['graphics'])
	conffile.write("\ntext=" + text)
        conffile.write("\nid=" + id)
        conffile.write("\nname=" + name)
	conffile.write("\nrarity=" + rarity)
#	conffile.write("\ncolor=" + color)
	conffile.write("\ntype=" + supertype)

        if manaCost:
            card.insert(2,Tag(xml,'attr'))
            card('attr')[2]['key']='cost'
            card('attr')[2]['value']=manaCost
	    conffile.write("\nmana=" + manaCost)
        if power:
            card.insert(len(card),Tag(xml,'attr'))
            card('attr')[-1]['key']='power'
            card('attr')[-1]['value']=power
	    conffile.write("\npower=" + power)
        if subtype:
    	    subtype = subtype.replace(u'\xe2', 'a')
            card.insert(len(card),Tag(xml,'attr'))
            card('attr')[-1]['key']='subtype'
            card('attr')[-1]['value']=subtype
	    conffile.write("\nsubtype=" + subtype)
        if toughness:
            card.insert(len(card),Tag(xml,'attr'))
            card('attr')[-1]['key']='toughness'
            card('attr')[-1]['value']=toughness
	    conffile.write("\ntoughness=" + toughness)
        card.insert(len(card),Tag(xml,'attr'))
        card('attr')[-1]['key']='type'
        card('attr')[-1]['value']=supertype

        printings += 1
        conffile.write("\n[/card]\n")
f = file(sys.argv[1],'w')
f.write(xml.prettify('latin1'))
f.close()
conffile.close()

