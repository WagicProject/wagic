# Changelog

## [master] (https://github.com/WagicProject/wagic/tree/master)

### 02/09/21
- *Committed:* Added/fixed primitives, updated the "missing_cards_by_sets" folder, fixed several crash on Commander Format, fixed a possible crash using "and!()!" with "imprint", added a new macros "_REBOUND_" and "_GOAD_" related to rebound and goad abilities and refactored all cards using them, implemented a new keywords "haunt", "hasprey", "preyname" and "isprey" related the haunting ability and improved all cards using it, Added "commander" and "\*" to HINT castpriority for AI decks in order to allow the user to give a cast priority to commanders. ([Vitty85](https://github.com/Vitty85))

### 31/08/21
- *Committed:* Added/fixed primitives, updated the "missing_cards_by_sets" folder, improved all cards with replicate (now they use a special version of multikicker but they don't count as kicked spell), improved all cards with "fizzle" and "fizzleto" ability and improved engine for "fizzle" and "nofizzle" ability (e.g. now it's possible to grant a card the nofizzle ability on stack), added new keyword "mycolnum" to count the number colors of a card. https://github.com/WagicProject/wagic/commit/54d0c3203551b377146d4bbe7d5af0c642b1058e ([Vitty85](https://github.com/Vitty85)) 

### 30/08/21
- *Committed:* Added/fixed primitives, updated the "missing_cards_by_sets" folder, added a new trigger when a player shuffles his/her library, added new keywords "plastshlturn" and "olastshlturn" to retrieve the last turn a player shuffled his/her library, refactoring of some source files. https://github.com/WagicProject/wagic/commit/45de20c8d3f0449e33286fad09b000a695b07c24 ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed dat file for J21 set, improved Android downloader for J21 set. https://github.com/WagicProject/wagic/commit/e1f91a50f6fa607ad3672433e00ceb1c25feb631 ([Vitty85](https://github.com/Vitty85)) 

### 29/08/21
- *Committed:* Fixed dat file for MB1, PRM and PSAL sets. https://github.com/WagicProject/wagic/commit/790f02905ac44cbcabd2fa7cd00ea11e6a36379c ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added/fixed primitives, fixed dat file of several sets, updated the "missing_cards_by_sets" folder, updated README.md file, improved Android downloader. https://github.com/WagicProject/wagic/commit/32008496b3016ebb0c8dc60e037c8ba7e6890802 ([Vitty85](https://github.com/Vitty85)) 

### 28/08/21
- *Committed:* Added/fixed primitives, improved the way to limit the number of total primitives to load at startup using the file named LimitedCardList.txt (that has to be copied in User folder). This should help to run the game even on older devices with low RAM such as PSP-3000 (It needs also a lighter graphics sub-folder in Res folder). https://github.com/WagicProject/wagic/commit/ebc281da6ed3f90a5c25f431458a563e16788017 ([Vitty85](https://github.com/Vitty85)) 

### 27/08/21
- *Committed:* Added/fixed primitives. https://github.com/WagicProject/wagic/commit/8d6e0122bb7253d29700bc4519067ed4160ddf84 ([Vitty85](https://github.com/Vitty85)) 

### 26/08/21
- *Committed:* Added/fixed primitives, Added keywords "showopponenthand" and "showcontrollerhand" to allow controller and opponent to play with their hand revealed, added "mytarg" prefix to check values for a card target as like we do for "storedcard" prefix (e.g. "Redirect"), fixed "undocpy" keyoword for all cards (e.g. "Renegade Doppelganger") that have to be back from a previous copy. https://github.com/WagicProject/wagic/commit/1ce0facf215f2af172e7e42b1f4d0ea25a7df38d ([Vitty85](https://github.com/Vitty85)) 

### 25/08/21
- *Committed:* Added/fixed primitives, Improved AI: now it can plays cards using morph cost too. https://github.com/WagicProject/wagic/commit/bbc25e2727b1007ecb28888ce26482d856187298 ([Vitty85](https://github.com/Vitty85)) 

### 24/08/21
- *Committed:* Added/fixed primitives, improved "TurnSide", "Morph" and "Flip" abilities when dealing with Commanders, added "fresh" attribute to cards just put in Sideboard, added a put back rule when a Commander is put in Sideboard, allowed to cast a card with kicker or alternative or morph cost from the CommandZone: in case of Morphed or DoubleFace cards (e.g. "Tergrid, God of Fright"), they will be put in play but they won't be Commander, but when they will be put elsewhere (e.g. destroyed) they may be back to the CommandZone with the usual Commander put back rule. https://github.com/WagicProject/wagic/commit/c7c2025fc9d44c4583a3e23a263824c3dcc62f59 ([Vitty85](https://github.com/Vitty85)) 

### 23/08/21
- *Committed:* Updated changelog with last 3 years of modifications (issue #1067 by @remigiusz-suwalski), added tokens in ELD set, improved Android downloader for ELD set, fixed primitives with "asflash" ability, improved all cards with adventure: now they become instants or sorceries in stack to activate the correct trigger (e.g. with Magecraft combos), added "nomovetrigger" ability for all the "fake" cards (e.g. cards which are simple abilities) in order to don't trigger any event on their cast since they are not real cards. https://github.com/WagicProject/wagic/commit/c978223b10f629f2b36f2e677254e293ec6aa39b ([Vitty85](https://github.com/Vitty85)) 

### 20/08/21
- *Committed:* Fixed "Cunning Rhetoric". (https://github.com/WagicProject/wagic/commit/cd9e5fb2e53e82dfb128b9a6110c76567af0ba0c) ([Vitty85](https://github.com/Vitty85)) 

### 19/08/21
- *Committed:* Fixed a crash on "Tevesh Szat, Doom of Fools" primitive. (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed "City's Blessing" macros, fixed "Jared Carthalion, True Heir", added new keyoword "noncombatvigor" for cards such as "Stormwild Capridor". (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added/fixed primitives, renamed "The Monarch" and "City's Blessing" cards in CN2 and RIX sets, added 2 new macros for Monarch abilites, refactoring of all cards with _ASCEND_ and _MONARCH_ macros. (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

### 18/08/21
- *Committed:* Added/fixed primitives. (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

### 17/08/21
- *Committed:* Added/fixed primitives and macros, fixed RIX and CN2 sets in order to allow images for Monarch and City's Blessing ebmlems, updated all cards with "Monarch" and "Ascend" related abilities, improved "token" keyword in order to allow the usage of "notrigger" option even when we are creating a named token, moved Monarch rules from general txt files to the specific Monarch emblem, improved Android downloader for RIX set. (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

### 16/08/21
- *Committed:* Fixed "Migratory Greathorn". (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added/fixed primitives and switched "Hound" type to "Dog" type. (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

### 15/08/21
- *Committed:* Fixed "Fabled Passage". (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed "Chance Encounte" and "Tavern Scoundrel". (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed "Shadowspear" and other primitives related to shroud abilities. (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

### 13/08/21
- *Committed:* Fixed J21 set, added/fixed primitives. (https://github.com/WagicProject/wagic/commit/7d465bfbc44db1e7941fe0f136b4c7fd882335ec) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Misspell Battalion (https://github.com/WagicProject/wagic/commit/b84ddc568780aa6ef63ac2b893aa807ae64ea158) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* New macros and primitives fixes. Fixes to whenever you draw your second card each turn, batallion (https://github.com/WagicProject/wagic/commit/b7f80f3851b2d05aa6ab5e3a1697749237143585) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 12/08/21
- *Committed:* Fixed "Pox" and "Blast Zone". (https://github.com/WagicProject/wagic/commit/a296db349c345e918bee7254d2ef00f75bf3c3b5) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added a way to limit the number of total primitives to load at startup using a file named LimitedCardList.txt (that has to be copied in User folder). This should help to run the game even on older devices with low RAM such as PSP-3000 and PSVita (with Adrenaline). (https://github.com/WagicProject/wagic/commit/1e0928b2277b2797dfab8f8025664128ce276b37) ([Vitty85](https://github.com/Vitty85)) 

### 11/08/21
- *Committed:* Fixed "Tevesh Szat, Doom of Fools". (https://github.com/WagicProject/wagic/commit/ecc5ddfe283315f4dc8350a5390d07bf44fcba52) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Changes to Random modes and improved filters. Improved search restriction by type, allow you to search for "enchantment creature" or "legendary artifact", the engine no longer excludes or prevents searching two "types". (https://github.com/WagicProject/wagic/commit/730ddd2b503978119c21be1fec600ad3fc9dee1d) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 10/08/21
- *Committed:* Fixed "Tergrid, God of Fright" and fixed some typos in primitives. (https://github.com/WagicProject/wagic/commit/b3b127ac2f600f4af37abbd8314ffd568757bc85) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added ZNR showcase cards, improved Android downloader for ZNR set, fixed primitives: "Tyrite Sanctum", "Tergrid, God of Fright" and "Liliana, Dreadhorde General". (https://github.com/WagicProject/wagic/commit/3219360cd7b1eab6021efbef6770f076e25d6e5e) ([Vitty85](https://github.com/Vitty85)) 

### 09/08/21
- *Committed:* Fixes to primitives (https://github.com/WagicProject/wagic/commit/cd55b4342c6d2164c320cf4f953e8a82a48faf13) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Fixed Plaguecrafter and Steel Hellkite. (https://github.com/WagicProject/wagic/commit/1727e11091898677fa81fecd29db8f474bbab6e5) ([Vitty85](https://github.com/Vitty85)) 

### 08/08/21
- *Committed:* Improved Android downloader for J21 set. (https://github.com/WagicProject/wagic/commit/9db4b9e2d444fc67f6505b79192e2b95a7f6c99b) ([Vitty85](https://github.com/Vitty85)) 

### 07/08/21
- *Committed:* Added/fixes primitives. (https://github.com/WagicProject/wagic/commit/b7e0fc6d8b4a997711db94ed9181c15230582c95) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added J21 set and added/fixed primitives. (https://github.com/WagicProject/wagic/commit/0f9b4c6ab2182bea899fe843737c4d3497b0880e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed "Jolrael, Mwonvuli Recluse", fixed "Corrosive Ooze", added new keyword "all(myeqp)" in order to target all the equipments attached to a creature (e.g. Corrosive Ooze). (https://github.com/WagicProject/wagic/commit/a82636b0995b97584656b030b966ba444ad8cbd1) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Macros for Enraged and Blocked (https://github.com/WagicProject/wagic/commit/b97bd275e4cc6c542117b555ce7f83aba9a9c5e8) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 05/08/21 
- *Committed:* Added/fixed primitives. (https://github.com/WagicProject/wagic/commit/b70e03a5cfcbb0996eecb21457044253050d0f1c) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Muxus, Goblin Grandee, fixed Time Wipe. (https://github.com/WagicProject/wagic/commit/946557acf2959c7b40abc04e0d18ff31dfe5199d) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Primitive fiexes and macros replacements (https://github.com/WagicProject/wagic/commit/f792729f8094439cfe30494afaa232e476463ba5) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 04/08/21
- *Committed:* Fix Mulligan when Human player is not the first one. (https://github.com/WagicProject/wagic/commit/4458a6e7808dcdfe5189a78861aab2903d751258) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Muxus, Goblin Grandee and fixed Liliana, the Last Hope, added conjure keyword for J21 set, added perpetual counters and abilities for J21 set, improved imprint keyword, improved moverandom keyword for J21 set. (https://github.com/WagicProject/wagic/commit/2ca03bb1f0a35b5a19a8b117b9107d1bcb2813be) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed MED and THB sets and fixed Liliana, the last Hope. (https://github.com/WagicProject/wagic/commit/e6ffd056b55c6f49ca9903f10410bc43f2b3adce) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Adding missing Planeswalkers to Mythic Edition and replaced code for _ATTACKING_ (https://github.com/WagicProject/wagic/commit/46ec7dc0c62089047fe704f4b17e1515ea914d03) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 03/08/21
- *Committed:* Fixed Flipwalkers and some macros replacements (https://github.com/WagicProject/wagic/commit/8ab49e021f4e0b4b218926929094ac851c25a9a5) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Fixes to primitives (https://github.com/WagicProject/wagic/commit/fb64821e0c33a46a775154b6dd5eb85a3d86cfed) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 30/07/21
- *Committed:* Fixed primitives. (https://github.com/WagicProject/wagic/commit/ef3e74398023210f096a4a65d6a56e0ce6d7388c) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixes to Primitives (https://github.com/WagicProject/wagic/commit/cffda7421f0391c9c3ebeabbf4f6cf069f211763) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 29/07/21
- *Committed:* Fixed issue #1065 by @DoidArthas: now on Flip keyword we try to keep auras and equipments effects on transformed card basic abilities (e.g. Pacifism on Werevolwf). (https://github.com/WagicProject/wagic/commit/bb5a9dd31ea4b2ed6f30073af809787d4a30db83) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added/fixed primitives, fixed crash with Taj-Nar Swordsmith, Fixed monarch switch after player takes combat damage, fixed allsubtypes in transforms keyword, fixed controllerdamager and opponentdamager keywords and all primitives using them, fixed Emerge alternative cost restriction, fixed Kinship condition on same creature subtype, added alterexperience keyword and fixed all primitives that use experience counters, fixed all primitives using ability counters from Ikoria, fixed mypoolsave keyword in order to don't finish effect at the end of turn, fixed all primitives using mypoolsave (e.g. Omnath, Locus of Mana), fixed Gravepurge and Daretti, Scrap Savant, in order to allow the player to discard zero cards while using their effects. (https://github.com/WagicProject/wagic/commit/bb5a9dd31ea4b2ed6f30073af809787d4a30db83) ([Vitty85](https://github.com/Vitty85)) 

### 27/07/21
- *Committed:* Improved Cursed Scroll and Magus of the Scroll, added/fixed primitives, solved Werewolf's Issues #1064 by @DoidArthas. (https://github.com/WagicProject/wagic/commit/d2fbaaf320989e8dc7d8369b4540dc739487d973) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixes to primitives and oracle text update (https://github.com/WagicProject/wagic/commit/1c9da2e6e6090e4c53b6b6c084b0823791271345) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 26/07/21
- *Committed:* Fixed promo sets order index. (https://github.com/WagicProject/wagic/commit/d8f1ff33ded0a87ce870779b1346f0c992252c15) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added PLG21 set, improved Android downloader, fixed Nicol Bolas, God-Pharaoh (issue #1063 by @DoidArthas) (https://github.com/WagicProject/wagic/commit/d8f1ff33ded0a87ce870779b1346f0c992252c15) ([Vitty85](https://github.com/Vitty85)) 

### 25/07/21
- *Committed:* Improved Cursed Scroll and Magus of the Scroll, fixed Android downloader. (https://github.com/WagicProject/wagic/commit/d8f1ff33ded0a87ce870779b1346f0c992252c15) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixes to primitives and useless comments removed (https://github.com/WagicProject/wagic/commit/9bdf0e0690d0904af3468a929f5df0b6769c7708) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Replacing for macros _DIES_ (https://github.com/WagicProject/wagic/commit/7e2162bcc3eb69db8c3adbe043bde3fd70309fc7) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 24/07/21
- *Committed:* Fixed primitives. (https://github.com/WagicProject/wagic/commit/57b8178d864f3fc447872f13d99686068c9f2bde) ([Vitty85](https://github.com/Vitty85)) 

### 22/07/21
- *Committed:* token(The Atropal) (https://github.com/WagicProject/wagic/commit/b2a4edae9f72100e2d692b73ff3c72d891a0f8fe) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Restore to previous modifications in borderline and bug fixes (https://github.com/WagicProject/wagic/commit/35a79cb72335c9d5b0def6b91a13c17c730cd1fd) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 21/07/21
- *Committed:* Fixed "The Atropal" token card in AFR set, improved Android downloader. (https://github.com/WagicProject/wagic/commit/005726936171e84d711d834643d1bc7ca8df2322) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added/fixed primitives, removed deckmaster.info source from Android downloader, fixed HTR18 set, fixed "menace" blocking issue from AI, improved Graft ability from AI, fixed hangs on Offering costs for both human and AI, added a new keyword "ishuman" to distinguish if a card controller is human or AI (e.g. on Graft trigger), fixed a possbile hang on negative manacost payment. (https://github.com/WagicProject/wagic/commit/005726936171e84d711d834643d1bc7ca8df2322) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Bug Fixes and replaced instances of a creature dying for the macros _DIES_ (https://github.com/WagicProject/wagic/commit/cabc0757bebcb4e3deed66d85f82242a2153c328) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 18/07/21
- *Committed:* Added/fixed primitives. (https://github.com/WagicProject/wagic/commit/23fa7f9f6eb32ff0d165cf98fae312b5952a4f56) ([Vitty85](https://github.com/Vitty85)) 

### 17/07/21
- *Committed:* Fix proliferate ability. (https://github.com/WagicProject/wagic/commit/23fa7f9f6eb32ff0d165cf98fae312b5952a4f56) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added HTR18, HT19, AFR and AFC sets, added/fixed old primitives, improved Android dowloader, fixed it lang file, fixed IMA rarities, fixed order index of some old sets, added all features from D&D such as Dungeon Cards and Dice (d20,d10, adn so on). (https://github.com/WagicProject/wagic/commit/23fa7f9f6eb32ff0d165cf98fae312b5952a4f56) ([Vitty85](https://github.com/Vitty85))

### 10/07/21
- *Committed:* Fixes to primitives (https://github.com/WagicProject/wagic/commit/d98956a1b81263dad65607bda0a68fa1a0c7eb5c) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 29/06/21
- *Committed:* Fixed Mana Drain (issue #1060 by @Gourajiro), fixed Reinterpret, fixed the taskboard menu choice during game. (https://github.com/WagicProject/wagic/commit/f0ee95b4834c94449ac8ff0d23b519ea6bd05b1a) ([Vitty85](https://github.com/Vitty85)) 

### 18/06/21
- *Committed:* Improved in-game menu (now the Cancel choice is on top and it's possibile to open/close the taskboard during game), improved add/remove "counter" keyword in order to avoid the trigger if needed (e.g. loop avoidance), fixed Italian and Spanish languages. (https://github.com/WagicProject/wagic/commit/f0ee95b4834c94449ac8ff0d23b519ea6bd05b1a) ([Vitty85](https://github.com/Vitty85))

### 17/06/21
- *Committed:* Added H1R set, added/fixed prmitives, improved Android downloader. (https://github.com/WagicProject/wagic/commit/5f0883943a1b4d51be2f91b96b4f41c448e2a108) ([Vitty85](https://github.com/Vitty85)) 

### 16/06/21
- *Committed:* Added MH2 set, improved Android downloader, added/fixed primitives, improved coin flip event trigger, improved discarded attribute, improved castcard keyword (now it's possible to specify the x value in cost), improved add/remove/mod counter trigger, improved the "spent" and "converge" keyword for spell still in the stack. (https://github.com/WagicProject/wagic/commit/5f0883943a1b4d51be2f91b96b4f41c448e2a108) ([Vitty85](https://github.com/Vitty85)) 

### 22/05/21
- *Committed:* Added HA5 set and imnproved Android downloader. (https://github.com/WagicProject/wagic/commit/5f0883943a1b4d51be2f91b96b4f41c448e2a108) ([Vitty85](https://github.com/Vitty85)) 

### 28/04/21
- *Committed:* Added a new rule to flip back modal dual face card on each phase and after each action, fixed "Aladdin's Lamp" and "Turntimber Symbiosis" primitives, allowed the AI to play back side of modal dual face cards, improved the "doubleside" keyword to flip modal dual face cards, improved filters to target flipped cards using the "isflipped" keyword, fixed a crash when zone pointer was null in GameObserver::logAction method. (https://github.com/WagicProject/wagic/commit/5f0883943a1b4d51be2f91b96b4f41c448e2a108) ([Vitty85](https://github.com/Vitty85)) 

### 27/04/21
- *Committed:* Fixed a bug: It's not allowed to turn side of double faced cards when they are on battlefield. (https://github.com/WagicProject/wagic/commit/ac273b194719130c0a9a463a816be92e05433a7c) ([Vitty85](https://github.com/Vitty85)) 

### 27/04/21
- *Committed:* Fixed a bug: AI does not have to use the doubleside ability to avoid loops. (https://github.com/WagicProject/wagic/commit/ac273b194719130c0a9a463a816be92e05433a7c) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added/Fixed primitives, improved the Double Face Modal cards management: now it's possibile to click on card to flip the side in odrer to read card infos such as name, manacost, text and types, improved the "moveto" keyword in order to allow the usage of the "temp" zone for removing unecessary cards from game (e.g. duplicated card generated from some dual face cards), added the option "nolegend" to the "copy" keyword in order to crerate copy of legendary cards that are not legendary (e.g. Echoing Equation), added the keywords "doublefacedeath" and "gaineddoublefacedeath" to send a card to temp zone after death (e.g. duplicated card generated from some dual face cards), added the keywords "lifefaker" to identify the cards wich modify the life increasement when a @lifeof triggers occours (e.g. Angel of Vitality). (https://github.com/WagicProject/wagic/commit/ac273b194719130c0a9a463a816be92e05433a7c) ([Vitty85](https://github.com/Vitty85)) 

### 26/04/21
- *Committed:* Completed STX set, fixed/added primitives, improved Android downloader, implemented the usage of ^ instead of , char (e.g. target multiple zones within transforms keyword), improved the code to avoid the multiple triggers in case of abilities gained from other cards (e.g. Kasmina, Enigma Sage). (https://github.com/WagicProject/wagic/commit/ac273b194719130c0a9a463a816be92e05433a7c) ([Vitty85](https://github.com/Vitty85)) 

### 21/04/21
- *Committed:* Added/fixed primitives, added new keywords "fourtimes", "fivetimes", "thirdpaid", fixed Tavis-CI build. (https://github.com/WagicProject/wagic/commit/ac273b194719130c0a9a463a816be92e05433a7c) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Travis-CI build. (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix error import QTOpenGL (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Travis-CI build (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Travis-CI build (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Travic-CI build. (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update travis-script.sh (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Try to change java version. (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/abe8b580c8eefe5cdf51b6a67d73914e19f0011e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Changed target and source java version from 1.5 to 1.6 (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Travis-CI build. (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Test Travis-CI compilation. (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Try to solve Travis-CI environment problems.

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/2a45b9f8df18fc66d4df68a5f7b8a0918bf863e7) ([Vitty85](https://github.com/Vitty85)) 

### 20/04/21
- *Committed:* Added C21, STA and STX (still in progress) sets, improved Android downloader, added/fixed primitives, added a keyword to get if a card has "X" in its cost, fixed a crash while targeting a spell on stack, added a new option "nolegend" to clone keyword in order to create a token without legendary type and rule, improved "hascnt" keyword with "anycnt" option to count all counters on a card, added a new keyword "hasstorecard" to get if a card has a stored card or not(e.g. fizzlers), added a new keyword "pgmanainstantsorcery" to count the mana value of all instants and sorceries in player graveyard, added a new keyword "currentphase" to get the current game phase. (https://github.com/WagicProject/wagic/commit/9da159a6078e3f081e07d095dddf0c3f355a7836) ([Vitty85](https://github.com/Vitty85)) 

### 12/03/21
- *Committed:* Added/fixed primitives. (https://github.com/WagicProject/wagic/commit/d7838829ad204c52fb060a73a012f8bb513db38f) ([Vitty85](https://github.com/Vitty85)) 

### 11/03/21
- *Committed:* Added/fixed primitives. (https://github.com/WagicProject/wagic/commit/d7838829ad204c52fb060a73a012f8bb513db38f) ([Vitty85](https://github.com/Vitty85)) 

### 11/03/21
- *Committed:* Added/fixed primitives, added a new ability "undamageable" (e.g. Dralnu, Lich Lord). (https://github.com/WagicProject/wagic/commit/d7838829ad204c52fb060a73a012f8bb513db38f) ([Vitty85](https://github.com/Vitty85)) 

### 09/03/21
- *Committed:* Added TSR and HA4 sets, added/fixed primitives, improved Android downloader, fixed a crash when AI pays a Convoke alternative cost, fixed a random crash on Windows when opening zip files, fixed a random crash in destroy ability method (e.g. blasphemous act). (https://github.com/WagicProject/wagic/commit/d7838829ad204c52fb060a73a012f8bb513db38f) ([Vitty85](https://github.com/Vitty85)) 

### 24/02/21
- *Committed:* Fixed Synthetic Destiny. (https://github.com/WagicProject/wagic/commit/c8f763173d801aa06f0e05bb979ae6c25145efa3) ([Vitty85](https://github.com/Vitty85)) 

### 23/02/21
- *Committed:* Improved AI: now during opponent's turn it will not search just for instant cards from hand but it can also play cards with "flash" ability from all the other available zones (e.g. "flash" cards from graveyards which have "canplayfromgraveyard" of Flashback and/or Retrace cost). (https://github.com/WagicProject/wagic/commit/c8f763173d801aa06f0e05bb979ae6c25145efa3) ([Vitty85](https://github.com/Vitty85)) 

### 22/02/21
- *Committed:* Improved AI: now it can plays cards using alternative cost too, fixed thisturn restriction in some primitives. (https://github.com/WagicProject/wagic/commit/c8f763173d801aa06f0e05bb979ae6c25145efa3) ([Vitty85](https://github.com/Vitty85)) 

### 19/02/21
- *Committed:* Fixed Hero of Bretagard. (https://github.com/WagicProject/wagic/commit/c8f763173d801aa06f0e05bb979ae6c25145efa3) ([Vitty85](https://github.com/Vitty85)) 

### 19/02/21
- *Committed:* Fixed a crash when AI try to activate a combo (caused by new ANYTYPEOFMANA management), fixed the Visual C++ project file descriptor, fixed ELD set DAT file, improved Andorid downloader, fixed primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 18/02/21
- *Committed:* Fixed some borderline primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved AI: now it can cast spells from graveyard using thier Flashback and Retrace costs, fixed a crash in Deck Editor stats loader when a card ability contains some "add" substrings (e.g. "counteradded" followed by "restriction{"), added/fixed primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 17/02/21
- *Committed:* Improved AI: now it can activate abilities of cards in all zones such as commandzone, hand, graveyard and exile using the keywords "autohand", "autocommandzone", "autograveyard" and "autoexile" just as normal Human player does. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 16/02/21
- *Committed:* Fixed all primitives with Fuse cost (now we use a special kicker cost for them), fixed all primitives with double kicker cost (now we use other and kicker cost togheter), added a new ability "hasnokicker" for primitives which have kicker for other purpose (e.g. Fuse cards), fixed an issue on cost name for both kicker and retrace cost, improved green highlight management for cards which can play in exile and graveyard (such as retrace cards, flashback cards, and so on), improved kicker cost management. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 15/02/21
- *Committed:* Fixed Lukka, Coppercoat Outcast. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Lukka, Coppercoat Outcas. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added the possibility to specify a name for Kicker cost and Retrace cost with "name()" keyword, implemented Escape cost with Retrace cost, implemented Fuse cost with Kicker cost, fixed several primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 13/02/21
- *Committed:* Fixed some primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 12/02/21
- *Committed:* Fixed primitives with "scry" and "reveal" abilities. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 12/02/21
- *Committed:* Added/fixed primitives, fixed/improved several primitives with "reveal" ability, added a new keyword "findfirsttype" to allow AI to user a kind of "revealuntil" ability from its library, fixed a crash wthen temp zone has duplicated cards from other zones (such as library). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 10/02/21
- *Committed:* Fixed all cards with "scry" ability, fixed all cards with "surveil" ability, fixed all cards with "explores" ability, fixed all cards with "adventure" ability. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 09/02/21
- *Committed:* Fixed cost reduction issue for card with X in their main cost or alternative cost. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

(https://github.com/WagicProject/wagic/commit/cf9f6ed474397436336261f541262035aed62f20) ([Vitty85](https://github.com/Vitty85)) 
### 08/02/21
- *Committed:* Fixed cost reduction issue for card with X in their cost. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed issue #1054 (opened by @ranger7271), fixed/added primitives with "scry" ability, improved scry ability for both Human and AI player, added a new ability to replace the scry ability with some actions (e.g. Eligeth, Crossroads Augur). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 06/02/21
- *Committed:* Fixed some primitives with "Suspend" ability. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 05/02/21 
- *Committed:* Fixed primitives with "preventalldamage from" ability. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed deck selection in Deck Editor Menu (now it shows both Classical Decks and Commander Decks using a CMD suffix) and fixed deck selection in Demo Mode (now it filters decks according to game mode as it happens in normal game mode). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 04/02/21
- *Committed:* Fixed some borderline primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed "anyzone" keyword: now it includes commandzone, reveal and sideboard too. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 02/02/21
- *Committed:* Added/fixed primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 01/02/21
- *Committed:* Fixed some borderline primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 31/01/21
- *Committed:* Fixed C18 set, fixed primitives from KHM set, improved Android downloader. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added KHM and KHC sets, improved Android downloader, fixed/added primitives, improved "hascnt" keyword, added "myhandexilegrave" and "opponenthandexilegrave" zone targeters. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 28/01/21
- *Committed:* Fixed Garruk's Harbinger and Gishath, Sun's Avatar (issue #1052 by @ranger7271). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Flamerush Rider, added battleready option to "clone with" keyword. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 27/01/21
- *Committed:* Fixed primitives, fixed a random crash in ActionStack.cpp, added a way to search wallpaers in theme folder before to search them in the default folder, improved the keyword to retrieve the highest power and toughness of creatures in play, now it's possibile to use "pwr:" and "ths:" instead of "power:" and "toughness:", so it's possibile to use this keyword in variable{} construct too. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 26/01/21
- *Committed:* Fixed Etchings of the Chosen (issue #1051 by @ranger7271), added primitives with choose card name, added two keywords "chooseaname" and "chooseanameopp" to choose a card name ("chosenname" and "lastchoosenname") between your cards or opponent cards, added a keyword "[attached]" to target equipment attached to a permanent. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 25/01/21
- *Committed:* Added a new keyword "excessdamage" to retrieve theamount of exceeded damage to creature or planeswalker, fixed an issue on planeswalker damage count, added a new keyword "genrand" to generate a random number between 0 and a specific number (e.g. "genrand3"), improved Flip ability in order to allow the flip back from copy for a generic card name (e.g. "flip(myorigname) undocpy)" (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 24/01/21
- *Committed:* Fixed Treasure token, added boast trigger event, added new keyword "hascnt" to retrieve the amount of specific counter type on a card (e.g. hascntloyalty). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed issues #1049 and #1050 opened by @ranger7271, improved imprint keywords, improved boast ability, added a new "hasability" keyword to check if a card has an ability or not. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 22/01/21
- *Committed:* Improved foretell mechanics, added a trigger for foretold cards, added a new keyword "snowdiffmana" to compare snow mana pool and mana cost of a target card, improved phaseaction "checkexile" condition. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed primitives, added "notshare!" keyword (e.g. to search cards with different names), improved decks selection according to chosen game mode (e.g. Game will show commander decks only in commander mode and it will hide them in other modes). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 21/01/21
- *Committed:* Added/fixed primitives, implemented a new keyword to count the greatest number creatures that share same subtype (creatures with changeling counts as +1 for all creature types) (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 20/01/21
- *Committed:* Fixed primitives, fixed multiple snow mana cost payments, added keywords to count snow mana pool (total and single colors). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed primitives, fixed multiple snow mana cost payments, added keywords to count snow mana pool (total and single colors). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 18/01/21
- *Committed:* Added boast ability, refactored WParsedInt class (this class has been removed from AllAbilities.h file and it has been included into two dedicated .h and .cpp files), refactored all makefiles (Windows, Linux, PSP and Android) in order to to include the new .h and .cpp files added for WParsedInt class. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 16/01/21
- *Committed:* Added/fixed primitives, improved "@draw" trigger, added a "@scryed" trigger for scry ability, added a new keyword "placefromthetop" to put a card in a specifc position of owners library from the top. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 15/01/21
- *Committed:* Added/fixed primitives, improved "anytypeofmana" ability for both player and AI and implemented "anytypeofmanaability" keyword to allow the user and the AI to spend mana of any color to activate abilities. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed/refactored primitives after the parser has been improved in order to allow the usage of "^" instead of "," char in a lot of abilities (e.g. to use the token, flip, rampage, phasealter, becomes keywords inside transforms or to target a card with a "," char in its name). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 14/01/21
- *Committed:* Fixed WAR, C18, PAL00, UST set, added/fixed primitives, improved Android downloader, implemented Foretell ability, improved castcard keyword, improved "can play" restriction, improved primitives parsed in order to allow the nesting of transforms, ability$! reveal, scry, pay, grant keywords. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 11/01/21
- *Committed:* Added/Fixed primitives, improved PAYZERO rule for commandzone. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 10/01/21
- *Committed:* Fixed MB1 set, added/fixed primitives, improved Android downloader, improved keywords to remove and add counters. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 09/01/21
- *Committed:* Added a new game option to allow the user to decide how to sort sets in filter and award section (by sector, by name or by release date). (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added ATH and DDN sets, refactored all sets to add a new tag for better sorting in set filter and award section, improved Android downloader. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 07/01/21
- *Committed:* Fixed/added primitives, added keyword to alter devotion count, added keyword to target cards with flashback cost, added "duplicatecounters(single)" keyword to add a counter of a specific kind already present on a permanent or a player. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 06/01/21
- *Committed:* Fixed Duplicant primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved the Imprint keyword ability and fixed Duplicant primitive. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed primitives. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 05/01/21
- *Committed:* Improved "can play land" restriction used for double-face modal cards from Zendikar Rising set. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 03/01/21
- *Committed:* Fix for IP Address resolution on Wondows for 2 Players mode. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed code trying to make the 2 Players mode work better. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 01/01/21
- *Committed:* Added missing graphics for game modes unlock, fixed italian lang, fixed awards dat file, fixed a bug on Game Award section, refactoring for AllAbilities.h file. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved and added new "starting life" related keywords, fixed all primitives related to "starting life" keywords. (https://github.com/WagicProject/wagic/commit/d3379c9c0852e0fbf3f23021d11ea2340df2ec1e) ([Vitty85](https://github.com/Vitty85)) 

### 31/12/20
- *Committed:* Fixed trophy image rendering for hi-res quality, fixed all primitives with XX cost, added some missing primitives from ol sets. (https://github.com/WagicProject/wagic/commit/e3aff3c23a5b70cf95dd364fba153797855f35d3) ([Vitty85](https://github.com/Vitty85))

### 30/12/20
- *Committed:* fixed bug causing AI never play a card there are one or more cards in graveyard or other zones, fixed and improved removemana ability, fixed/added primitives. (https://github.com/WagicProject/wagic/commit/1444ed6b7c4c4f9fafc9836028a4ae5164d5a780) ([Vitty85](https://github.com/Vitty85)) 

### 27/12/20
- *Committed:* Avoided a memory allocation error of SDL EnginePlayer on Android version when loading more than 32 audio samples at same time. (https://github.com/WagicProject/wagic/commit/1f9817319745434134b97a96728821482a53782b) ([Vitty85](https://github.com/Vitty85)) 

### 26/12/20
- *Committed:* Fixed Italian and Spanish lang, fized Commander award condition. (https://github.com/WagicProject/wagic/commit/b7c6725b443c589faabd90e2c27b5de15bf87466) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Increased Android SDK and NDK version for Android 10, updated Italian and Spanish lang files, patch boost dependencies for TIME_UTC definition, fixed import for usleep. (https://github.com/WagicProject/wagic/commit/f9e0746e1c0ad4610188bdea0e56232444fddbda) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Updated spanish lang, Improved Commander rules and award, fized/added all primitives with divide damage between one or more targets. (https://github.com/WagicProject/wagic/commit/b2c55e307c217139cb93ad4fa1d9d7e26b9e91a1) ([Vitty85](https://github.com/Vitty85)) 

### 25/12/20
- *Committed:* Fixed Edgar Markov, issue #1047 (https://github.com/WagicProject/wagic/commit/99033e3fb22fc027682697222248e8cfce3acb2c) ([Vitty85](https://github.com/Vitty85)) 

### 25/12/20
- *Committed:* Fixed primitives with imprint ability. (https://github.com/WagicProject/wagic/commit/053f202f32f8f7348f89002593bd0b42c60f65c7) ([Vitty85](https://github.com/Vitty85)) 

### 24/12/20
- *Committed:* Fixed Duplicant primitive. (https://github.com/WagicProject/wagic/commit/991625dea460dfb494602119497306b1b03d2f15) ([Vitty85](https://github.com/Vitty85)) 

### 23/12/20
- *Committed:* Added feature to allow some cards to copy and flip back at the end of turn, added feature to use type: keyword with chosentype and chosencolor combo, fixed crashes on "can play land" restriction, fixed land primitives with pay life or tap condition to avoid crashes. (https://github.com/WagicProject/wagic/commit/0ca310da545f8986b8703879495f0d76886f3e24) ([Vitty85](https://github.com/Vitty85)) 

### 22/12/20
- *Committed:* Fixed/added primitives from older sets. (https://github.com/WagicProject/wagic/commit/d40d6f319e6845d39f46a970ed932a5ea496711e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed all primitives with Domain ability, fixed issue #1044 opened by DroidArthas. (https://github.com/WagicProject/wagic/commit/3a8d70ff272a27fb7ff118e6f1f049b09fd1886f) ([Vitty85](https://github.com/Vitty85)) 

### 21/12/20
- *Committed:* Fixed and improved the Deck Importer (from MTGO) in Android application, fixed frozen and freeze untap bug, fixed/added primitives. (https://github.com/WagicProject/wagic/commit/1aed1c1517ab09cee689f43f8c6898927cf3f67a) ([Vitty85](https://github.com/Vitty85)) 

### 20/12/20
- *Committed:* Improved explores ability, added new trigger to handle the explores event from a card, added/fixed primitives from older sets, fixed RIX dat file. (https://github.com/WagicProject/wagic/commit/a801069a3ab45d030e9a8fdce5486218a3acfcf3) ([Vitty85](https://github.com/Vitty85)) 

### 18/12/20
- *Committed:* Improved target chooser to allow a multiple selection for player, creature and planeswalkers, now it's possible to use target(player,creature,planeswalkers) as well as target(<variable>player,creature,planeswalkers) and so on. Added abilities for giving exiledeath and handdeath to instant and sorceries (the previous were not working fine), Added ability to identify the cards with Cycling ability as a target, Updated all primitives with cycling ability, fixed all primitives with choose any target ability, Added/fixed primitives from RIX set. (https://github.com/WagicProject/wagic/commit/e67078355a489d729c32501aa104f290625fab48) ([Vitty85](https://github.com/Vitty85)) 

### 16/12/20
- *Committed:* Added new keyword to count creature of the same type with both unique and common names, added/fixed primitives form M19 set. (https://github.com/WagicProject/wagic/commit/0d2e4af2a306810f6369281680de3a8a8c0f0462) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added abilities for cards which have to return battlefield or hand instead of graveyard (e.g. just like exiledeath ability), improved fresh attribute management also for card moved in hand, library, commandzone and for instants and sorcery spells, added/fixed primitives. (https://github.com/WagicProject/wagic/commit/3e7ef8c77c04eec3e950a6c0c1ca7ed8960f620d) ([Vitty85](https://github.com/Vitty85)) 

### 15/12/20
- *Committed:* Improved Die Roll event and trigger, added Flip Coin trigger management, added/fixed almost all primitives with "roll a die ability", fixed some tab chars in source files. (https://github.com/WagicProject/wagic/commit/b09763d89e8514dfa81806a49fb84e265300c1e0) ([Vitty85](https://github.com/Vitty85)) 

### 14/12/20
- *Committed:* Added commander mode achievement and improved its graphic resources, added fixed primitives, fixes RNA set file, added new keyword and events to code the ability of six-side die roll. (https://github.com/WagicProject/wagic/commit/bf3d35463f88428028de6e1f45115a2ed3539946) ([Vitty85](https://github.com/Vitty85)) 

### 11/12/20
- *Committed:* Implemented new keyword for creatures which have to be blocked from 3 or more other creatures, added/fixed primitives. (https://github.com/WagicProject/wagic/commit/a753bb0c1b426808d43844f429e17fafe4ea008c) ([Vitty85](https://github.com/Vitty85)) 

### 10/12/20
- *Committed:* Added/fixed some primitives. (https://github.com/WagicProject/wagic/commit/d2ea81bdbcef9d1b45382d635182bdecfcf2427d) ([Vitty85](https://github.com/Vitty85)) 

### 09/12/20
- *Committed:* Fixed compilation issue on AllAbilities.h (https://github.com/WagicProject/wagic/commit/d4b663220fe404c61294c6c50087728b93e7cc38) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix compilation issue. (https://github.com/WagicProject/wagic/commit/f4353fb0e9dc9fcb869141c916004995473ff817) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added/fixed primitives. (https://github.com/WagicProject/wagic/commit/3ec955bf2ce009149b42890ab225d98e13b45a9e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added/fixed primitives and implemented new keywords to count the single color symbols in player's manapool. (https://github.com/WagicProject/wagic/commit/68d21ddd23adece286eef0a0a7b30ba80cd7e725) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added CC1 set, added/fixed primitives related to commander abilities, fixed cost increase for Commanders who have been cast multiple times. (https://github.com/WagicProject/wagic/commit/16e6df95032fc230a027395193e46ea909d2583d) ([Vitty85](https://github.com/Vitty85)) 

### 08/12/20
- *Committed:* Improved Commander format card selection rule, added new keyword "autocommandzone=" to trigger abilities from Command Zone (e.g. Commander Ninjutsu), added/fixed some primitives, added a new GUI Button to show Player's Sideboard. (https://github.com/WagicProject/wagic/commit/7c696ac2132942ed37017) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Commander Format selection of identity color, added/fixed primitives related to commanders abilities. (https://github.com/WagicProject/wagic/commit/d18232df1043f2c01d86dfe4564246c3ff5bd36f) ([Vitty85](https://github.com/Vitty85)) 

### 07/12/20
- *Committed:* Added/fixed some primitives, improved Deck Editor to allow user to choose commanders from collection and add them to their decks. Implemented command color identity rule and single card instance limitation for Commander Format game mode. (https://github.com/WagicProject/wagic/commit/d6a1a8eda8a85692b872aa70f84cf39926e04dfa) ([Vitty85](https://github.com/Vitty85)) 

### 06/12/20
- *Committed:* Implemented Commander mode and rules, added Command Zone to game, added/fixed primitives, increased the major release version. (https://github.com/WagicProject/wagic/commit/ced2c850766e20dae8b3c746fbedf52f9208e06b) ([Vitty85](https://github.com/Vitty85)) 

### 01/12/20
- *Committed:* Fixed/added some primitives. (https://github.com/WagicProject/wagic/commit/6ef9ecc147633ef1da352ebaadaf3ad304111f08) ([Vitty85](https://github.com/Vitty85)) 

### 30/11/20
- *Committed:* Added CMR set, added Monarch game mode, added/fix several primitives and improved Android downloader. (https://github.com/WagicProject/wagic/commit/c704dfbfb2da2f5ed2c592b63fc09712bf86a847) ([Vitty85](https://github.com/Vitty85)) 

### 19/11/20
- *Committed:* Fixed and added some primitives. (https://github.com/WagicProject/wagic/commit/d751ed3540bf35ae9a7657f7d95dfb638e41555c) ([Vitty85](https://github.com/Vitty85)) 

### 18/11/20
- *Committed:* Added KLR set, fixed some primitives and improved Android downloader. (https://github.com/WagicProject/wagic/commit/1202ea609eb3a39ad9d197f6075a4838b938f8de) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed a crash occurring on token clone (e.g. Doubling Season) as described in the issue #1040. (https://github.com/WagicProject/wagic/commit/252e66736a9b8b9d3021a275df637ca361708023) ([Vitty85](https://github.com/Vitty85)) 

### 27/10/20
- *Committed:* Changed implementation strategy for altercosat in anyzone for some cards due to some problem. (https://github.com/WagicProject/wagic/commit/42f2c34a464e73340516bb58c8d9fe59ecc6efd2) ([Vitty85](https://github.com/Vitty85)) 

### 21/10/20
- *Committed:* Fixed Cursed Scroll primitive. (https://github.com/WagicProject/wagic/commit/c78e90a5b5e80b10922ee1714e3aa11662638bf2) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added Cursed Scroll primitive. (https://github.com/WagicProject/wagic/commit/a52cd305b7cb68b797924dc2ab4a9d865fb363e4) ([Vitty85](https://github.com/Vitty85)) 

### 20/10/20
- *Committed:* Added EasyPBPRX program to the release to sign PRX up to 8Mb size. (https://github.com/WagicProject/wagic/commit/6a00c0ad1d4d5a468fa5780eb2fc28770ce72ecd) ([Vitty85](https://github.com/Vitty85)) 

### 19/10/20
- *Committed:* Added PLIST set and missing primitives, improved Android downloader and fixed all primitives with Adapt ability. (https://github.com/WagicProject/wagic/commit/e6c1f652dbe1fe59cec2afdcd7ed1d6b7d2577f0) ([Vitty85](https://github.com/Vitty85)) 

### 18/10/20
- *Committed:* Fixed The Great Henge primitive. (https://github.com/WagicProject/wagic/commit/48bdc6c9516dce5c6148b1079d183da191b38466) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed all primitives with Escape cost, fixed token creation (the movedto triggers were not activating correctly due to a bug). (https://github.com/WagicProject/wagic/commit/939dc20855e15a683fdddd4130fb9434c55bdecc) ([Vitty85](https://github.com/Vitty85)) 

### 16/10/20
- *Committed:* Removed FTP transfer due to many failures detected. (https://github.com/WagicProject/wagic/commit/8c39eac856d90d314efc08549ea7cf4e285aad5a) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Modified the timeout for FTP transfer. (https://github.com/WagicProject/wagic/commit/272199dced2af53606b0b45733721609f1a0951b) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Set timeout for FTP transfer. (https://github.com/WagicProject/wagic/commit/cab90b8fe22740d340059f478b027caa3eb992e4) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Modal Double Faced Lands of Zendikar Rising set. (https://github.com/WagicProject/wagic/commit/d5a8b80f5aebb28e25df3f55449aa109afb5f1b7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added a python script to upload Travis-CI build files on external FTP server. (https://github.com/WagicProject/wagic/commit/4f3a179be47dbf6dea4da884712a44979a22128b) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Modal Double Faced Lands of Zendikar Rising set. (https://github.com/WagicProject/wagic/commit/519c90dd9d84f9c19736ade8251480db3b5f70ff) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed travis.yml file for Travis-CI build. (https://github.com/WagicProject/wagic/commit/3d46b41efce4d9d036bda06f0b7b9fb2a8186452) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Updated the python script to upload the Travis-CI build results to a specific GitHub release. (https://github.com/WagicProject/wagic/commit/fbf1f9e1e0ca1bf78086257f083bfc55c6ecfb73) ([Vitty85](https://github.com/Vitty85)) 

### 15/10/20
- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/d0cd6e8b2134d8053200a9ed0379a01d8e7b8304) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/90c51fbb2dc210118137de4f8ccd30d9f3a0c2a1) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/edf7714f0ca51b8cbe2f5708065a024759e68b04) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml, Updated GitHub variables (https://github.com/WagicProject/wagic/commit/77241ee496bc89158c496742f8464a678bc6fbbd) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml (https://github.com/WagicProject/wagic/commit/ad0e8383e4fbb15a5048b2b6c2633584cc4b75c7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml, test secure variables (https://github.com/WagicProject/wagic/commit/795debeb0d29c1b53c8d23ea86d5834e1dfa50ac) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml, Test with new token and github user (https://github.com/WagicProject/wagic/commit/b7a7efd9db99a5795141eb8bc427b08fd24fb8c4) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update .travis.yml, test variables (https://github.com/WagicProject/wagic/commit/d005aefdc7f12dbf97ce236363f3b1f197664259) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Archfiend's Vessel and Orah, Skyclave Hierophant primitives. (https://github.com/WagicProject/wagic/commit/30647615f2e4c44c07ee79b2cd10e11d75912b09) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Relic Viel sacrifice ability. (https://github.com/WagicProject/wagic/commit/3ee3aaf7d801f3d57c244358860265bbfd7ad19c) ([Vitty85](https://github.com/Vitty85)) 

### 14/10/20
- *Committed:* Fixed Enhanced Surveillance primitive. (https://github.com/WagicProject/wagic/commit/a2403e4eee23869af9b8040f4f9d3a7e6d991fd2) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Update README.md (https://github.com/WagicProject/wagic/commit/3e618c062afd907ae850a82250fa3c4048a00b36) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Switched badge from travis-ci.org to travis-ci.com (https://github.com/WagicProject/wagic/commit/b1bc13cb0d5abe6bf0bca57efc35a86f00bc6dfc) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Fix mana cost of Relic Vial. (https://github.com/WagicProject/wagic/commit/fe60f5fe81e230c30bbb98ed1c7d4c77b212c19d) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed all the Modal Double Faced card from Zendikar Rising set, now we don't use the alternative cost anymore but the autohand keyword instead which required another enhancement for Flip forcetype keywords, fixed a bug on Changezone trigger. (https://github.com/WagicProject/wagic/commit/7bf463c7eab1ec77cc9c360c28b453a2fc2b0433) ([Vitty85](https://github.com/Vitty85)) 

### 13/10/20
- *Committed:* Improved Life gain trigger (now it's possible to specify a "from" clause and it's possible to add the "limitOnceATurn" restriction) and fixed all the primitives with gives life on life gain event without producing any loop and reviewed all the primitives with the "@lifeof" trigger. (https://github.com/WagicProject/wagic/commit/e1df4e5072a341af32db9b3bbc64770a86cbcefb) ([Vitty85](https://github.com/Vitty85)) 

### 12/10/20
- *Committed:* Fix primitive from Zendikar Rising set. (https://github.com/WagicProject/wagic/commit/1b860218e0a845b1c5e2194430e4e926fa18c7be) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed a crash on counterremoved trigger, fixed/add some primitives that have ability to double the damage to any target. (https://github.com/WagicProject/wagic/commit/75347a2f3771f673d466e4ce90b5779af6512347) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added missing cards from GRN sets, improved mutation trigger, improved Surveil ability, implemented trigger and offset on surveil ability to allow combo with other cards (see issue #1037 opened by luisguerin). (https://github.com/WagicProject/wagic/commit/29985718a75b97886cee45c7758f48a431717643) ([Vitty85](https://github.com/Vitty85)) 

### 11/10/20
- *Committed:* Fixed primitives list contained in issue #1037 opened by luisguerin. (https://github.com/WagicProject/wagic/commit/8cf2b7ab69ac544596f76ffd72ddb01352a26a69) ([Vitty85](https://github.com/Vitty85)) 

### 10/10/20
- *Committed:* Improved kicker cards comparison criteria (now it's possible to search for a multi kicked card in stack), added a new castcard mode with multikicker option, added all Zendikar Risings primitives to borderline collection, changed in all primitives the restriction "kicker" with a new sintax "if paid(kicker) then" in order to fit with the new kicker logic comparison criteria and castcard option. (https://github.com/WagicProject/wagic/commit/a99eaac35dd487cd4897fbd91e6a3774818c51c6) ([Vitty85](https://github.com/Vitty85)) 

### 09/10/20
- *Committed:* Fixed primitives and planeswalkers, fixed a bug on counter comparison criteria when cards are changing zone (eg. @movedto(creature[counter{1/1}]|mygraveyard) from(mybattlefiled) now it's working fine). (https://github.com/WagicProject/wagic/commit/c4eb93119284b2d04c372c79ec44c77952cf2011) ([Vitty85](https://github.com/Vitty85)) 

### 08/10/20
- *Committed:* Improved the Modal Double Faced cards for Zendikar set, added primitives that deals x damage divided on any target, added/fixed planeswalkers and improved kicker cost event handling. (https://github.com/WagicProject/wagic/commit/a6a053e10e3b0c007e86da9dfc852f5c170f8ba2) ([Vitty85](https://github.com/Vitty85)) 

### 06/10/20
- *Committed:* Improved Android downloader, fixed snow mana cost, merged opponentlifetotal and oplifetotal keywords (https://github.com/WagicProject/wagic/commit/ad26450151274301dbb7a31d90fed124e0f5e302) ([Vitty85](https://github.com/Vitty85)) 

### 05/10/20
- *Committed:* Fixed typos in some primitive's restrictions. (https://github.com/WagicProject/wagic/commit/e4f5d7784a1f69ed5b55c1bbd86dbf7c80f7927d) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed and added cards with kicker, multi-kicker, replicate and strive costs. (https://github.com/WagicProject/wagic/commit/023816aa654b28aaa3227f85e3c320bbc9a311df) ([Vitty85](https://github.com/Vitty85)) 

### 03/10/20
- *Committed:* Improved Kicker cards, now it's possible to target a specific card with kicker cost and handle any event connected to a kicker casting cost. (https://github.com/WagicProject/wagic/commit/bf6439db00710ab8c1960a9d05c05b7b44c293ea) ([Vitty85](https://github.com/Vitty85)) 

### 01/10/20
- *Committed:* Fixed party counter routine and removed tabbed chars from cpp file. (https://github.com/WagicProject/wagic/commit/1b320d532581fa9ae45c732685a3f234abe7fb82) ([Vitty85](https://github.com/Vitty85)) 

### 30/09/20
- *Committed:* Improved the Modal Double Faced cards for Zendikar Rising (es. nofizzle when choosing land face, flip for instant and sorcery, new restriction for playing land face during turn). (https://github.com/WagicProject/wagic/commit/6d872ed176f43ca914f7209f0695bea7619e7732) ([Vitty85](https://github.com/Vitty85)) 

### 29/09/20
- *Committed:* Fixed primitives and tokens, implemented count for party of creatures in Zendikar cards. (https://github.com/WagicProject/wagic/commit/00ce669edbd0700d678ef73a92444eb9bd58f5e9) ([Vitty85](https://github.com/Vitty85)) 

### 27/09/20
- *Committed:* Added ZNR, ZNE, ZNC sets, fixed some primitives and improved Android downloader. (https://github.com/WagicProject/wagic/commit/61dc3013f451a82f09b8604149d89c0f58b7bc66) ([Vitty85](https://github.com/Vitty85)) 

### 21/09/20
- *Committed:* Fixed primitives, fixed alias 1117 and fixed the bug on lastCardDrawn from library. (https://github.com/WagicProject/wagic/commit/c4ba879382336ef361cf3177971e15296c081092) ([Vitty85](https://github.com/Vitty85)) 

### 20/09/20
- *Committed:* Fixed primitives. (https://github.com/WagicProject/wagic/commit/495e5f142966f5415a4df6910ae8d616eeb3b6ed) ([Vitty85](https://github.com/Vitty85)) 

### 19/09/20
- *Committed:* Fixed primitives. (https://github.com/WagicProject/wagic/commit/827f5a71bc02a65bdba16d2fa4b05614e8780bc6) ([Vitty85](https://github.com/Vitty85)) 

### 16/09/20
- *Committed:* Merged with last master commit, fixed primitives and languages. (https://github.com/WagicProject/wagic/commit/573c72f72d5e6e2bd8e2f29ae62eba1b1cc668f0) ([Vitty85](https://github.com/Vitty85)) 

### 10/09/20
- *Committed:* Changes to some game modes I created and fixes to primitives, Some minor changes to some game modes, by the way, can we make it easier to unlock the random modes? (https://github.com/WagicProject/wagic/commit/612be9737d37f1b0df9aa3c2b6f45271580b5092) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 07/09/20
- *Committed:* Fixed primitives and planeswalkers. (https://github.com/WagicProject/wagic/commit/4bc339e82beb641df4ab4e7fa83d0c9232f9c374) ([Vitty85](https://github.com/Vitty85)) 

### 31/08/20
- *Committed:* Fixed and added macros for primitives. (https://github.com/WagicProject/wagic/commit/668fc41862c8040a0d99bcb0975ddcda61cd20a2) ([Vitty85](https://github.com/Vitty85)) 

### 28/08/20
- *Committed:* Merge branch 'master' of https://github.com/WagicProject/wagic (https://github.com/WagicProject/wagic/commit/457b8c3bf2a5bd85c3f4fbfc66ed9a532a4845a9) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Bug fixes on primitives, borderline and planeswalkers (https://github.com/WagicProject/wagic/commit/135f2a1e458b80a882fdc797b82839d31fc6927c) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 26/08/20
- *Committed:* Fixed some primitives and merged with last GIT commit. (https://github.com/WagicProject/wagic/commit/2d9cab915a89858601f18780aef3cdf8484d2d96) ([Vitty85](https://github.com/Vitty85)) 

### 25/08/20
- *Committed:* Merge branch 'master' of https://github.com/WagicProject/wagic (https://github.com/WagicProject/wagic/commit/22d8d16b7136758894de47e1bb4d878ed9e03eec) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 15/08/20
- *Committed:* Added AKR and ANB sets, Fixed Aftermath primitives and DAT files for AKH and HOU sets, Added extended art cards to 2XM set and improved Android downloader. (https://github.com/WagicProject/wagic/commit/453906e35030deed18d7cebc64338a72bfb709ea) ([Vitty85](https://github.com/Vitty85)) 

### 11/08/20
- *Committed:* Fixes to borderline cards mostly and to rules.cpp (https://github.com/WagicProject/wagic/commit/5e16e0d1bafa71c3aebcc2682785eeb258a3babb) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 10/08/20
- *Committed:* Fixed and added some card prmitives. (https://github.com/WagicProject/wagic/commit/1c099182255da9b609c8d4797695ba8c8083d3c6) ([Vitty85](https://github.com/Vitty85)) 

### 09/08/20
- *Committed:* Fixed tokens and planeswalkers primitives. (https://github.com/WagicProject/wagic/commit/251e1634f72a07ad5583653bbb05cf9e8fc87812) ([Vitty85](https://github.com/Vitty85)) 

### 08/08/20
- *Committed:* Added 2XM set, fixed card and planeswalkers primitives, fixed a crash on token creation, fixed prowess and mentor abilities, added new restrictions, improved Android downloader. (https://github.com/WagicProject/wagic/commit/727d4579a0121cc47216a9e0c3186e838e2663c2) ([Vitty85](https://github.com/Vitty85)) 

### 10/07/20
- *Committed:* Added new features and triggers to game about tokens and counters, fixed and added new primitives. (https://github.com/WagicProject/wagic/commit/52e3177ef2a94864f868f35903c63c18b7d36cdf) ([Vitty85](https://github.com/Vitty85)) 

### 04/07/20
- *Committed:* Added JMP, SSR and M21 sets, fixed C14 set, added new primitives and improved Android downloader. (https://github.com/WagicProject/wagic/commit/fc40971dc7d255d9899bf920f540d259c781db43) ([Vitty85](https://github.com/Vitty85)) 

### 24/06/20
- *Committed:* Fixed crash on Dread Presence and added new primitives. (https://github.com/WagicProject/wagic/commit/ccc26f400d4a221b64f4137bcd7da889f0be0578) ([Vitty85](https://github.com/Vitty85)) 

### 22/06/20
- *Committed:* Added new primitives and added a new menu choice to toggle all creature to attacking mode during attack phase. (https://github.com/WagicProject/wagic/commit/210a250dce8c44304b9c9efb831d8e3f9066e84d) ([Vitty85](https://github.com/Vitty85)) 

### 15/06/20
- *Committed:* Fix X cost for Alternative payment, added new primitives. (https://github.com/WagicProject/wagic/commit/a2594f1fb7d52960159f0295e8a405f128ae0541) ([Vitty85](https://github.com/Vitty85)) 

### 14/06/20
- *Committed:* Fixed Legend rule for mutating cards, avoided spell targeting for mutated down cards, added new primitives. (https://github.com/WagicProject/wagic/commit/587155353caeb261fb215bd4748d2ef85d848696) ([Vitty85](https://github.com/Vitty85)) 

### 12/06/20
- *Committed:* Fixed Sliding Menu for Android App, added new primitives and fix mutating cards. (https://github.com/WagicProject/wagic/commit/2b398b425de2605838d869c688173d0f696ae374) ([Vitty85](https://github.com/Vitty85)) 

### 11/06/20
- *Committed:* Fix !share!types! keyword (https://github.com/WagicProject/wagic/commit/67c710268764556f764ea4485f8c263ade0cd97e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix !share!types! keyword (https://github.com/WagicProject/wagic/commit/6212af0f484a7f6eea464158ecf487d722a2940f) ([Vitty85](https://github.com/Vitty85)) 

### 10/06/20
- *Committed:* Fix !share!types! problem for creatures with no subtypes (https://github.com/WagicProject/wagic/commit/5688b289818e267ea83e455d86d3333b4f791985) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix build error to_string (https://github.com/WagicProject/wagic/commit/c162eff8cffef49e3d9c5623829efc3126cb8bf0) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed some primitives and fixed a problem with share!types! keyword. (https://github.com/WagicProject/wagic/commit/60e6d314e066e3856cf7f1a538d4c3b921edf613) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix VERSION_GAME macro for 3 params. Error on build. (https://github.com/WagicProject/wagic/commit/7cf722114ba59f50d3f7f3ddb388a8a0732d1fae) ([Vitty85](https://github.com/Vitty85)) 

### 10/06/20
- *Committed:* Fix date in Dat files, Added IKO and HA3 sets, updated the manifest and build files, Added SD card support for Android, Fix the Android app crash when resuming from background, Improved Android downloader, added finger sliding popup menu for Android devices without sidebar menu, fixed several crashes during game, added the mutating card ability, fixed the adventure card ability, added new borderline primitives. (https://github.com/WagicProject/wagic/commit/8645cb9e1ea26042bafac87e2b8cae3fe77f0f2d) ([Vitty85](https://github.com/Vitty85)) 

### 09/05/20
- *Committed:* Merged and sorted all primitives in their respective files. (https://github.com/WagicProject/wagic/commit/275eb9e06e70f3e6665eae4f5506013db176040e) ([Vitty85](https://github.com/Vitty85)) 

### 08/05/20
- *Committed:* Added more C20 primitives. (https://github.com/WagicProject/wagic/commit/60b4bb31f5995bd4a1ba6d0e6d58ac463fe141f5) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed C20 set file and improved Android downloader. (https://github.com/WagicProject/wagic/commit/a5eb2eba376c99936b869e5a9cd323c8c372460e) ([Vitty85](https://github.com/Vitty85)) 

### 19/04/20
- *Committed:* Added borderless images support on Android card images downloader. (https://github.com/WagicProject/wagic/commit/a00cf2a1d11b95d7ecb48dcea7fa83b60c9c33fb) ([Vitty85](https://github.com/Vitty85)) 

### 16/04/20
- *Committed:* Fixed card types and subtypes visualization, added C20 set and improved Android downloader. (https://github.com/WagicProject/wagic/commit/b863188c3cadfed0c2ea3d2421a27c2c61001dcf) ([Vitty85](https://github.com/Vitty85)) 

### 30/03/20
- *Committed:* Fix primitives with Escalation keyword. (https://github.com/WagicProject/wagic/commit/21778de9fbd2f01ce2b0ac87180d30264b836f65) ([Vitty85](https://github.com/Vitty85)) 

### 29/03/20
- *Committed:* Fix on primitives with Explores and Escalation keywords. (https://github.com/WagicProject/wagic/commit/c81eecf6d29dc03652ca4db6c3e45e0a76c34f7d) ([Vitty85](https://github.com/Vitty85)) 

### 28/03/20
- *Committed:* Fix primitives with Explores and Escalation keywords. (https://github.com/WagicProject/wagic/commit/402323a990b70d1fc26f148887b6a46ba1f62bb6) ([Vitty85](https://github.com/Vitty85)) 

### 23/03/20
- *Committed:* Fixed some primitives. (https://github.com/WagicProject/wagic/commit/75b335e59c5201542b3ae789327a6cf861cf7e5e) ([Vitty85](https://github.com/Vitty85)) 

### 17/03/20
- *Committed:* Added HA2 set and improved Android downloader. (https://github.com/WagicProject/wagic/commit/9f69336c94e4f2ed1410916388847894a6804a0b) ([Vitty85](https://github.com/Vitty85)) 

### 12/03/20
- *Committed:* Added THB and UND sets, improved Android image downloader. (https://github.com/WagicProject/wagic/commit/331e54a73290e88aad45017e98b40145a933047c) ([Vitty85](https://github.com/Vitty85)) 

### 11/12/19
- *Committed:* Added MB1 set and improved Android downloader. (https://github.com/WagicProject/wagic/commit/3d25630dd62bf6db7d4db31de96dfc3763bc1ef3) ([Vitty85](https://github.com/Vitty85)) 

### 09/12/19
- *Committed:* Removed MB1 set. (https://github.com/WagicProject/wagic/commit/5b652ba6886b77498810d4d1d31a3aacbec65aaf) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed MB1 set and improved Android downloader. (https://github.com/WagicProject/wagic/commit/c60df6c99d6d1fddeed3f5a964a53d57a96e3485) ([Vitty85](https://github.com/Vitty85)) 

### 08/12/19
- *Committed:* Added MB1 set and improved Android downloader. (https://github.com/WagicProject/wagic/commit/8bef671a707d1a90bd3cc588b3164dc271c73dfa) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Reverted .travis.yml (https://github.com/WagicProject/wagic/commit/9df8e90ab6b5e0c91cee8caa2c0eb92efde64aa4) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Test Travis secure vars. (https://github.com/WagicProject/wagic/commit/45d7cecaf31adc70fe43e1a58044050d352f9488) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Test travis secure vars. (https://github.com/WagicProject/wagic/commit/730c8479f988d82ff7d2e10d1df07ee189d86d35) ([Vitty85](https://github.com/Vitty85)) 

### 06/12/19
- *Committed:* Fixed Android Downloader. (https://github.com/WagicProject/wagic/commit/1dca08a33ba1a2a0cd75e94387e62b84e675965f) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added CN2, ME1 and SLD set, fixed primitives, fixed test and improved Android Downloader. (https://github.com/WagicProject/wagic/commit/233cec7a9f56db0e3cb568db82f86bafd18ebcc0) ([Vitty85](https://github.com/Vitty85)) 

### 22/11/19
- *Committed:* Added HA1 set, fixed primitives and improved Android downloader. (https://github.com/WagicProject/wagic/commit/08f69dc832dd1244b4b6ecca9bd9654f7ccae094) ([Vitty85](https://github.com/Vitty85)) 
 
### 15/11/19
- *Committed:* Fixed GN2 set/primitives and improved Android downloader. (https://github.com/WagicProject/wagic/commit/464a6ff1b893a2bfff5f2f72d28a1d625f80e392) ([Vitty85](https://github.com/Vitty85)) 

### 09/11/19
- *Committed:* Removed duplicated card in Guild Kit sets and improved Android downloader. (https://github.com/WagicProject/wagic/commit/c66476035927b929158571451f9e36687d238802) ([Vitty85](https://github.com/Vitty85)) 

### 08/11/19
- *Committed:* Added GK1_AZORIU, GK1_BOROS, GK1_DIMIR, GK1_GOLGAR, GK1_IZZET, GK1_SELESN, GK2_GRUUL, GK2_ORZHOV, GK2_RADKOS, GK2_SIMIC, GN2, PAL00, PAL01, PAL02, PAL03, PAL04, PAL05, PAL06, PAL99, PARL sets, improved Android downloader, added new primitives and updated the release version to 0.22.2 (https://github.com/WagicProject/wagic/commit/f180b8cc565d14d698c37b63b8d198995bcb3909) ([Vitty85](https://github.com/Vitty85)) 

### 04/11/19
- *Committed:* Fixed MH1 set and primitives. (https://github.com/WagicProject/wagic/commit/044bbb3d4b6da125e809590288f7b66370db3c6b) ([Vitty85](https://github.com/Vitty85)) 

### 03/11/19
- *Committed:* Added DPA, PDP10, PDP11, PDP12, PDP13, PDP14, PMPS, PMPS06, PMPS07, PMPS07, PMPS08, PMPS09, PMPS10, PMPS11 sets and improved Android downloader. (https://github.com/WagicProject/wagic/commit/c4884e4715c83b0c43cf43ef76be41835debbe51) ([Vitty85](https://github.com/Vitty85)) 

### 01/11/19
- *Committed:* Fixed Flaxen Intruder primitives of ELD set. (https://github.com/WagicProject/wagic/commit/253be9e9bd2cd5b33c8ad189d4ac1f1a7ff6df2c) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added PS11, PSAL sets and improved Android downloader. (https://github.com/WagicProject/wagic/commit/db517858fad07c4b042ac1cefa3dc67ad580b8e1) ([Vitty85](https://github.com/Vitty85)) 

### 25/10/19
- *Committed:* Fixed primitives. (https://github.com/WagicProject/wagic/commit/923a8e8fd9ab4c302d5140c80ed7ead760b203a2) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed ELD set. (https://github.com/WagicProject/wagic/commit/8d395620a11cfea6c228e78b12405b40e9ff19f7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added ANA, H17, HTR, HTR17, MPS, PGRU, PHPR, PI13, PI14, PIDW and improved Android downloader. (https://github.com/WagicProject/wagic/commit/a54759160834273fbdb9141ee50d4aaa5e4cba5b) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added new option to show/hide card borders. (https://github.com/WagicProject/wagic/commit/601a4432dc9c404c76ce470dcf676cd0a9a5f369) ([Vitty85](https://github.com/Vitty85)) 

### 24/10/19
- *Committed:* Removed outer border from rendered cards. (https://github.com/WagicProject/wagic/commit/ca7c8f5cf9eca17

### 22/10/19
- *Committed:* Added PZ2 set and improved Android Downloader. (https://github.com/WagicProject/wagic/commit/f44ad4341369d821cb57b13b77d430e6e51a609a) ([Vitty85](https://github.com/Vitty85)) 

### 13/10/19
- *Committed:* Fixed ELD Adventure primitives. (https://github.com/WagicProject/wagic/commit/1ec448daef51aa86bfab2313d59e3df79827dbc8) ([Vitty85](https://github.com/Vitty85)) 

### 12/10/19
- *Committed:* Fix GRN and RIX primitives. (https://github.com/WagicProject/wagic/commit/adee84d60f4ea4e8da8b93e162df359cf4e8cc35) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed a GRN primitive. (https://github.com/WagicProject/wagic/commit/a46786eb2854c9f4611a8e05d4bb72fbd340bb55) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed ELD primitive. (https://github.com/WagicProject/wagic/commit/5c7b4b76188827bb204a43c44df75375089baf7e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed primitives of GRN and ELD sets. (https://github.com/WagicProject/wagic/commit/bdcb10a54fb3ca73feb3089d96f2b1523d8c9cec) ([Vitty85](https://github.com/Vitty85)) 

### 10/10/19
- *Committed:* Fixed Midnight Reaper primitive in GRN set. (https://github.com/WagicProject/wagic/commit/6083ad76d6491f2219bf034fc3b628865ed6e592) ([Vitty85](https://github.com/Vitty85)) 

### 08/10/19
- *Committed:* Fixed Crash on Dread Presence deletion, Fixed primitives for ELD set. (https://github.com/WagicProject/wagic/commit/b66674492b982ff0813c00a06862abede7ad03f0) ([Vitty85](https://github.com/Vitty85)) 

### 07/10/19
- *Committed:* Fixed M20 primitives. (https://github.com/WagicProject/wagic/commit/0f0e627cf51fdffed0f47379583e1f89c87f3bb8) ([Vitty85](https://github.com/Vitty85)) 

### 06/10/19
- *Committed:* Fixed primitives of ELD, GRN and M20 sets. (https://github.com/WagicProject/wagic/commit/83e7bfffd06c688e52cf70bd2189f8e8147e2132) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed primitives for ELD and XLN sets. (https://github.com/WagicProject/wagic/commit/28b1c0e86f82a83972a9a30f3303a37fcbbbba07) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed M20 primitives. (https://github.com/WagicProject/wagic/commit/d0e4aa635b9958501b6e00b599fa2489b41e9462) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed some primitives for M20 and ELD sets. (https://github.com/WagicProject/wagic/commit/a82a2e35718bcb1a3d24a4d49923e8c412f1c7c1) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added new primitives for ELD set. (https://github.com/WagicProject/wagic/commit/68161b2aaae1c4654defe74544230f2cd158c7a5) ([Vitty85](https://github.com/Vitty85)) 

### 05/10/19
- *Committed:* Added PZ1 set and improved Android downloader. (https://github.com/WagicProject/wagic/commit/4e7463c77eacf23f94b99ad82976f64182a1b082) ([Vitty85](https://github.com/Vitty85)) 

### 04/10/19
- *Committed:* Added new primitives for PRM and ELD sets, removed some duplicated primitives. (https://github.com/WagicProject/wagic/commit/8ab7212aa20ea22fea82c94e19adfea4c05fdc38) ([Vitty85](https://github.com/Vitty85)) 

### 01/10/19
- *Committed:* Added ELD set and improved Android downloader. (https://github.com/WagicProject/wagic/commit/16d989e8cac2b345df49d3dc422f78d3d5288921) ([Vitty85](https://github.com/Vitty85)) 

### 27/09/19
- *Committed:* Added PRM and TD0 set and Improved Android Downloader.(https://github.com/WagicProject/wagic/commit/0c8e51693a19880414619f90465d9ac25a842c2d) ([Vitty85](https://github.com/Vitty85)) 

### 25/09/19
- *Committed:* Fixed image name search bug on Android Downloader. (https://github.com/WagicProject/wagic/commit/9f4e7fb7b22e1e52073b0811f5ccf825bcb239ff) ([Vitty85](https://github.com/Vitty85)) 

### 22/09/19
- *Committed:* Added TPR set and improved Android Downloader. (https://github.com/WagicProject/wagic/commit/9dd5807caec9031a289b3537366867f850a2e272) ([Vitty85](https://github.com/Vitty85)) 

### 21/09/19
- *Committed:* Added VMA set and Improved Android Downloader. (https://github.com/WagicProject/wagic/commit/7441ae2e8b3087ef0374f7022038aba4629d741d) ([Vitty85](https://github.com/Vitty85)) 

### 20/09/19
- *Committed:* Fix Yawgmoth, Thran Physician (https://github.com/WagicProject/wagic/commit/c8869f4048221d62464a2eeb6731229fd7c7e7d1) ([Vitty85](https://github.com/Vitty85)) 

### 19/09/19
- *Committed:* Fix Android Downloader. (https://github.com/WagicProject/wagic/commit/21440c977d5551005e8a36618d8579e182c0cff7) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added GS1 set and Improved Android Downloader. (https://github.com/WagicProject/wagic/commit/06ccc923eaf262543d6b0c014a01fa00cf3423be) ([Vitty85](https://github.com/Vitty85)) 

### 17/09/19
- *Committed:* Added GNT set and Improved Android Downloader (https://github.com/WagicProject/wagic/commit/c82fc3b5ceddb86f89168ac69e1a7377f576b9e1) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix Android Downloader, Added zombie to 10E set, fixed Liliana the Last Hope. (https://github.com/WagicProject/wagic/commit/7a19ae8f403f48dbd929bac42b57daa7aa8c158b) ([Vitty85](https://github.com/Vitty85)) 

### 16/09/19
- *Committed:* Fixed Loxodon Lifechanter (https://github.com/WagicProject/wagic/commit/43be15e83263cbbd0cdf5464a555a7882578b4d4) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Liliana of the Veil, Bloodthirsty Aerialist and Twinblade Paladin. Improved Android Downloader (https://github.com/WagicProject/wagic/commit/4ce2a5c36e301cb91f4e3e98f4fcd58220a5104d) ([Vitty85](https://github.com/Vitty85)) 

### 14/09/19
- *Committed:* Added MD1/V16/W16 sets and updated Android Downloader. (https://github.com/WagicProject/wagic/commit/82e53a8c39b084ba3d5cb76d0df8604adb8624e8) ([Vitty85](https://github.com/Vitty85)) 

### 13/09/19
- *Committed:* Fix Resources zip filename on Android downloader. (https://github.com/WagicProject/wagic/commit/dcc7e23b84cb9516ae8973872a22884c1cb79213) ([Vitty85](https://github.com/Vitty85)) 

- *Merge-pull-request:* Merge pull request #1027 from Vitty85/master (https://github.com/WagicProject/wagic/commit/54e4d881a743a011206e81fe026dc72e5a977a31) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added TD2 set, increased Wagic version to 0.22.1, fixed some bugs in Android Java wrapper. (https://github.com/WagicProject/wagic/commit/54e4d881a743a011206e81fe026dc72e5a977a31) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added TD2 set, increased Wagic version to 0.22.1, fixed some bugs in Android Java wrapper. (https://github.com/WagicProject/wagic/commit/83f8d1c5d7e252a2416ef682f825e989fdfc2494) ([Vitty85](https://github.com/Vitty85)) 

### 12/09/19
- *Merge-pull-request:* Merge pull request #1026 from Vitty85/master (https://github.com/WagicProject/wagic/commit/cf738e76d31bca7d6abfde712fea8cba86f85013) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Removed almost all duplicated primitives fixed life gain loop adding new keyword "except", added PCA/W17/DDR sets and merged the PSP graphic Folder with PC/Android one (https://github.com/WagicProject/wagic/commit/cf738e76d31bca7d6abfde712fea8cba86f85013) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Added DDR set and improved Android Downloader. (https://github.com/WagicProject/wagic/commit/744e380df9da2e6fcb88bfe1b91e42f6e4d7f51a) ([Vitty85](https://github.com/Vitty85)) 

### 11/09/19
- *Committed:* Added W17 set (https://github.com/WagicProject/wagic/commit/88e09bbd0540dee757bc529394475190f8094b53) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* fix missing bracket (https://github.com/WagicProject/wagic/commit/7d865781520451aacfac580c084f86ac1d5c1abb) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added several ifdef PSP for different resolution image to merge the graphics folders. (https://github.com/WagicProject/wagic/commit/6fd6f9061c76a61619a73d82fb1eb158696a3fcc) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* merged the graphics dir with PSP one, modified code with ifdef PSP in order to load different resolution images for different devices. (https://github.com/WagicProject/wagic/commit/ba918b27eefdd11dad2254b25105c129438bcce1) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Correctly renamed PSP menu background image for deck selection (https://github.com/WagicProject/wagic/commit/0b90a471d6832a295b7ff960c9f5d7268baf05db) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Should fix broken QT build (https://github.com/WagicProject/wagic/commit/8970fa673be75622bb47236f1b8b299217c80602) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Removed mac specific build files (https://github.com/WagicProject/wagic/commit/4e4752856b0767b38a3cd402138e624b2921da55) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Added a build specific for mac (https://github.com/WagicProject/wagic/commit/bc79115985b9ef7ca8b795f55338386a12d7d512) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Removes -qt=qt5 for osx (https://github.com/WagicProject/wagic/commit/29b89ba562dd035461c29befb7662bbba3951c80) ([Xawotihs](https://github.com/Xawotihs))

### 10/09/19
- *Committed:* Added testcases for new keyword except used by Angel of Vitality. (https://github.com/WagicProject/wagic/commit/24fd17ecf85e34003f1842c090366a77ff6582eb) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Angel of Vitality in M20 set and added new keyword "except" for @lifeof trigger in order to avoid a life gain loop when a card needs to increment life gaining in that phase (eg. Angel of Vitality). (https://github.com/WagicProject/wagic/commit/06ec97676e3d1602772d0feba6993dddbc4c2ef8) ([Vitty85](https://github.com/Vitty85)) 

### 09/09/19
- *Committed:* Added Vitty85 to credits. (https://github.com/WagicProject/wagic/commit/6ecd762d183d4b2aed49ab1e171767e8bf8edf51) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed Angel of Vitality primitive (still a bit buggy but not looping) (https://github.com/WagicProject/wagic/commit/0fa1f04e7133b7842055b2f7b2193f9b9c420446) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added PCA set to Resources (https://github.com/WagicProject/wagic/commit/225be4d9aada25d0296a26bd91d26a238cb91d77) ([Vitty85](https://github.com/Vitty85)) 

### 08/09/19
- *Committed:* Fix Glacial Revelation primitives in MH1.txt (https://github.com/WagicProject/wagic/commit/07119b88f6937597b3bd4d2d62c0263213c8d930) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Removed almost all duplicated primitives (https://github.com/WagicProject/wagic/commit/0132f7aa5effea34ba073c40cbbef09c4c47d70f) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Sorting the qmake  path (https://github.com/WagicProject/wagic/commit/7d51b985a6b78098d5f712feb3d3e282fac7d7f4) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* we build res if we build psp, removed the ref to the mac script (https://github.com/WagicProject/wagic/commit/91bfa11a4ffbc67041e90b58038fa1ea745b9635) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Let's build the res zip only if needed (https://github.com/WagicProject/wagic/commit/46834a6ee9be099a6009db792b7feb425d556ff9) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* installing ant on osx as well (https://github.com/WagicProject/wagic/commit/95af76a3b20a7a54031a5996d6ff5ff50dd4e784) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Fied typo for osx (https://github.com/WagicProject/wagic/commit/4df34c82c26adb6b0b898bd58ec8ecb1a02a6f71) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Merge branch 'master' into matrix (https://github.com/WagicProject/wagic/commit/50d49e47e73094db885574257097fb042fee5ad5) ([Xawotihs](https://github.com/Xawotihs))

- *Merge-pull-request:* Merge pull request #1025 from Vitty85/master (https://github.com/WagicProject/wagic/commit/87ce6df145b1d09af8e3a979d15236e9d920ed00) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Fix for PSP/Android and added new sets M20/MH1/SS2/C19 and fixed UST/V17 sets (https://github.com/WagicProject/wagic/commit/87ce6df145b1d09af8e3a979d15236e9d920ed00) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Removed duplicated Tireless Tracker (https://github.com/WagicProject/wagic/commit/f5a20110c4f8e0c9f60da6ef977400dbe33c5227) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Recompilef libs wtih JDK 1.7 (https://github.com/WagicProject/wagic/commit/264183a42ae284a874d919511c73702f2828d174) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Set travis toolchain android target to 23 (https://github.com/WagicProject/wagic/commit/af0de566fe94c6d0cd0eabfb031ad9b832e81455) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Target android back to 29 (https://github.com/WagicProject/wagic/commit/480adfe83a9a4bc8687cb35f752878155defbc78) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Removed java.util.stream dependency because not supported in Travis CI Android compiler. (https://github.com/WagicProject/wagic/commit/ce5b07b5b1f48457e44b442e5df770824e43d2e2) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Removed dependencies from java.nio package and decreased target android version from 29 to 13 (https://github.com/WagicProject/wagic/commit/09098893e61ad5a9656e94266e484899f56ddc8d) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix Prismatic Vista card (https://github.com/WagicProject/wagic/commit/414d99525d1d82efe5be8457f0238150b3fb968f) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Should fix a typo (https://github.com/WagicProject/wagic/commit/b29c37eb25666e38574b24874cdfffdfd44ad843) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* First try on matrix without cmake (https://github.com/WagicProject/wagic/commit/c7d09d838920240cc14d06b571897d32b5e32b32) ([Xawotihs](https://github.com/Xawotihs))

### 07/09/19
- *Committed:* Restored 29 target platform for Android (https://github.com/WagicProject/wagic/commit/4f42ddadb53e2fdc3a1e2a750eb778b388a5eb8c) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Revert TIME_UTC_ to TIME_UTC because Travis CI compile errors. (https://github.com/WagicProject/wagic/commit/901a7ca98434d6fa2b1beb78943ae349ab3d1cba) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* restored gnustl_static flag for android compile since travis toolchain doesn't support c++_static flag (https://github.com/WagicProject/wagic/commit/d3feccff75b52a1f319883491b5ccb43508adc5e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Restored Android version to 16, Removed modification to boost, fixed primitives MH1 and M20, Fixed Snow mana issue. (https://github.com/WagicProject/wagic/commit/ccd17d32d05a93e7a6b1acacea689c2460685067) ([Vitty85](https://github.com/Vitty85)) 

### 06/09/19
- *Committed:* fix spaces (https://github.com/WagicProject/wagic/commit/5c67d11aa656d3d330f0fd3a7a5f1af10641b7bd) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Merge branch 'master' into master (https://github.com/WagicProject/wagic/commit/7a529bac6d2c4f8f2d4b7beb68d2cad7c1b41958) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* replaced tabs with 4 spaces. (https://github.com/WagicProject/wagic/commit/2c62cfd487d1f9d43dc46bb7f49f1a18253e2e32) ([Vitty85](https://github.com/Vitty85)) 

### 05/09/19
- *Committed:* Deactivating mac/ios build as currently broken (https://github.com/WagicProject/wagic/commit/a27216c31731257df94f01b0850f35d4a3a27e01) ([Xawotihs](https://github.com/Xawotihs))

### 04/09/19
- *Committed:* Removed death_grasp.txt and ai/goblin_artillery.txt from the testsuite (see issue#1023) (https://github.com/WagicProject/wagic/commit/8315acbe8625458e5c5a73461ea4509d37e537a3) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Using trusty dist as it seems the only one supported for Android. (https://github.com/WagicProject/wagic/commit/7a2f23db4529cb09eec3941ecb1677fe810be7ba) ([Xawotihs](https://github.com/Xawotihs))

### 03/09/19
- *Committed:* Updating JDK to 7 (https://github.com/WagicProject/wagic/commit/711f52e5ce15073b681f0a55999acc4939c3187d) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Installing ant for android build (https://github.com/WagicProject/wagic/commit/0cf8c1a97035a75a88e46dd58e712baa7dcb34dd) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Trying to fix python dependencies (https://github.com/WagicProject/wagic/commit/2750b34b4fe0bdaf8953779c6295078ec59ae4bc) ([Xawotihs](https://github.com/Xawotihs))
 
- *Committed:* - Should remove an error with the PSP compiler (https://github.com/WagicProject/wagic/commit/b023dd89b6a472aa7a132c6c19069ad7c7eb100b) ([Xawotihs](https://github.com/Xawotihs))

- *Committed:* Fix 2 cards mana cost in MH1 set. (https://github.com/WagicProject/wagic/commit/91f50009e19b9d9934616ef283880adde8afab4e) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix on token image saving in Android Downloader (https://github.com/WagicProject/wagic/commit/9504027fdc7ac63e17413c35701aa73d4a621f70) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix timeout on image database download. (https://github.com/WagicProject/wagic/commit/5c7528dbcffdd1566f009c88bab9356f4abe5c9a) ([Vitty85](https://github.com/Vitty85)) 

### 02/09/19
- *Committed:* Added retries on image and database file download for Android Downloader program. (https://github.com/WagicProject/wagic/commit/e856538259869a9662b283a0b188a84cef1c4a28) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix on some token images. (https://github.com/WagicProject/wagic/commit/1749330e8a64dfc052e6a010dd709c7cf09cc013) ([Vitty85](https://github.com/Vitty85)) 

### 01/09/19
- *Committed:* Fix on 439538t token image (https://github.com/WagicProject/wagic/commit/893e64e4f7f8d6f8ac6d0a7f589cb626f31867ac) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix for some duplicated token on Android Image Downloader. (https://github.com/WagicProject/wagic/commit/169467158a2fa481b1f4e8fcc34f08e4bf634235) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Set socket timeout on Android Downloader and fixed some minor exceptions. (https://github.com/WagicProject/wagic/commit/ab0a5496b0b813a82c08a93682419d97a0287d8b) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix names of Tokens in Android Image downloader. (https://github.com/WagicProject/wagic/commit/0c4deea3edec1d235de1dbf2570f7426535c111c) ([Vitty85](https://github.com/Vitty85)) 

### 31/08/19
- *Committed:* Fix some token names on Android Downloader (https://github.com/WagicProject/wagic/commit/dd785a872e46cafabe997089263de98e309d11df) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Removed one comment on old IFDEF PSP (https://github.com/WagicProject/wagic/commit/c8603204ac928fe1077768dd5611e0cd3e506f32) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed a bug on set name when using the fast download method (https://github.com/WagicProject/wagic/commit/ae81568463271e621a19ba5ea40c0b27d0c456ae) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved the Android downloader, now it's a lot faster because all the image urls have been indexed. (https://github.com/WagicProject/wagic/commit/2fe9924fe56b6fd38e357073e8027f5b4c8fc54a) ([Vitty85](https://github.com/Vitty85)) 

### 29/08/19
- *Committed:* Fix on UST card download image (https://github.com/WagicProject/wagic/commit/818233213ea4d4c2d4406b3dcaeb795bc616bb5b) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix on some indexes on image donwloader. (https://github.com/WagicProject/wagic/commit/d9019c080825c6f9726fa2b951b15db412e57edc) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed V17 set total card numbers from 16 to 29 in dat file. Improved Android card image downloader. (https://github.com/WagicProject/wagic/commit/fdaed6503a15ef1b9ff17e4d809cbd47c7b21b24) ([Vitty85](https://github.com/Vitty85)) 

### 28/08/19
- *Committed:* Improved Android Downloader: Bug Fixing and boosted token searching algorithm. (https://github.com/WagicProject/wagic/commit/e10213a373078f838fcc0758e6fdeb4f4f8585d9) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Android downloader: the token searching algorithm has been boosted in terms of speed and reliability. (https://github.com/WagicProject/wagic/commit/34732a5f5427763d537a147a91f9a5a995db2d63) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fixed minor bugs on downloader. (https://github.com/WagicProject/wagic/commit/d8d6025652fea6ebd53fe98a2ee391a60a3b7d18) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Token searching algorithm and fixed some image URLs with higher resolution from DeckmMster and ScryFall. (https://github.com/WagicProject/wagic/commit/01117f4cb72590d626d14930b1e9152d2f536927) ([Vitty85](https://github.com/Vitty85)) 

### 27/08/19
- *Committed:* Improved downloader, added pause/resume. (https://github.com/WagicProject/wagic/commit/a25448cde7e48222411f5124c925c90fa25ecdba) ([Vitty85](https://github.com/Vitty85)) 

### 26/08/19
- *Committed:* Fix on UST set for Android Downloader and fix on StopDownload action. (https://github.com/WagicProject/wagic/commit/ef4b18b394dbe696a18106e77bfdb519779989da) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Android Downloader, fix on UST set. (https://github.com/WagicProject/wagic/commit/683b2fbed87885fde90a0f90dce0e11fee448bf4) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Android Downloader and bug fixing (https://github.com/WagicProject/wagic/commit/cdfadec38ba22c7e68bc8cbee991211cb579ab4c) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Android Downloader: added Image Resolution chooser and fixed several bugs. (https://github.com/WagicProject/wagic/commit/7b434f33e809f8406e7a3d850c5f1e1bf24c3dd0) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Android Downloader and added the multiple set selection in the download option menu. (https://github.com/WagicProject/wagic/commit/29f1420abcdbeadba10e50a7b1372de141a4bde5) ([Vitty85](https://github.com/Vitty85)) 

### 25/08/19
- *Committed:* Improved Dowloader: Set Async thread and displayed a progress bar for resource loading and card image downloading. (https://github.com/WagicProject/wagic/commit/39884d9711e4ab242bd1f978e8663f722c5de170) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Downloader and Option Menu handlers. (https://github.com/WagicProject/wagic/commit/a5c9f877f7e129ca97101cd9604de739d5dbfcd5) ([Vitty85](https://github.com/Vitty85)) 

### 24/08/19
- *Committed:* Improved Android Image Downloader adding execptions and errors handling (https://github.com/WagicProject/wagic/commit/c603765908db750853e44be2841df2ed9eedd827) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Android Image Downloader (https://github.com/WagicProject/wagic/commit/a0b4a1f77c8904be9ce21d663a1728d222c30646) ([Vitty85](https://github.com/Vitty85)) 

### 23/08/19
- *Committed:* Improvement on token search algorithm, not it even uses deckmaster website. (https://github.com/WagicProject/wagic/commit/e0a2a45037022069bdfbd9c1f4090b20326f18a2) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved token search algorithm. (https://github.com/WagicProject/wagic/commit/4c6f5126a6c026ddc1451603048b8bbedc47d318) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix on C19 _cards.dat file (added double cards such as dusk/dawn, etc.) (https://github.com/WagicProject/wagic/commit/50da2fef421be42532045eeabdfa4e5a94e454a9) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix on Android downloader and code indent (https://github.com/WagicProject/wagic/commit/9dfd513ad3ae6cf4e78d2c634bc6f4529ea0967a) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix Exception on Android downloader. (https://github.com/WagicProject/wagic/commit/30622010dff22951b0cae7ae1935b870f5b35451) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Added the C19 set and primitives (https://github.com/WagicProject/wagic/commit/f4d9de94901e10325566542458dcdba7ebf173b3) ([Vitty85](https://github.com/Vitty85)) 

### 22/08/19
- *Committed:* Improved Downloader adding retries and a better token search algorithm. (https://github.com/WagicProject/wagic/commit/1092d4121a14f5024cd728194fd9b9327992496d) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved the Image Downloader (now Zip works at the end of download!) (https://github.com/WagicProject/wagic/commit/2ec4b78a8acefca7f7402b5ce1f0f5c73af0a403) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Improved Image Downloader (but still zipper not working on Android) (https://github.com/WagicProject/wagic/commit/1b88f7d3f98e41249b0ca78b25d5a7b2b22a6397) ([Vitty85](https://github.com/Vitty85)) 

### 20/08/19
- *Committed:* Added SS2 set. (https://github.com/WagicProject/wagic/commit/2ac3ec6f09fced4586e7ca57b86f0ccc66fc6d9a) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Downloader for Card Images now working on Android version with OptionMenuButton (no zip) (https://github.com/WagicProject/wagic/commit/73138d21760d6a4a6f3c40c5258aea6139ee6fc2) ([Vitty85](https://github.com/Vitty85)) 

### 19/08/19
- *Committed:* Retored Option Menu Android and added a draft of card images downloader (not working yet) (https://github.com/WagicProject/wagic/commit/7cb0d8b84355788733226a3121bfe422431dc2f1) ([Vitty85](https://github.com/Vitty85)) 

### 18/08/19
- *Committed:* Separated graphics of PSP from PC/Android version (https://github.com/WagicProject/wagic/commit/1dc5c48ff6e4f8e609cefa6a8d285e2710450664) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Fix on Primitives and Fix Android (https://github.com/WagicProject/wagic/commit/42754fd83c4d5c77fa857fc33109716df1894552) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Android Fix (https://github.com/WagicProject/wagic/commit/301de0f6f77cce6c6a89cde48ed98afbf0d78566) ([Vitty85](https://github.com/Vitty85)) 

- *Committed:* Android Fix (https://github.com/WagicProject/wagic/commit/9be1d447882ddd5c284463f4170314ba3915cef4) ([Vitty85](https://github.com/Vitty85)) 

### 10/08/19
- *Committed:* Added PSP res file, fixed bug for PSP version and added M20, MH1 and UST sets and primitives. (https://github.com/WagicProject/wagic/commit/7328c45013c6ede1e8e793b911d9eaad13c48415) ([Vitty85](https://github.com/Vitty85)) 

### 08/05/19
- *Committed:* Commander Anthology (CMA) Correction (https://github.com/WagicProject/wagic/commit/3d9526a1dd1f4294f18278192e8c06a29400544f) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 06/05/19
- *Committed:* Merge branch 'master' of https://github.com/WagicProject/wagic (https://github.com/WagicProject/wagic/commit/1b8157ef0e55d0a5cb65b753628f06572eb7b7e7) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Fixes to primitives (https://github.com/WagicProject/wagic/commit/8d20fd64838511fa61c04301ca60d4617b13a1ea) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Revert "Fixes to primitives". This reverts commit eb7d8850f4f99e4064b4cd7dda82920d7f07d603. (https://github.com/WagicProject/wagic/commit/66741957c36bb8eb79f2c2f3e6735274011db497) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Fixes to primitives (https://github.com/WagicProject/wagic/commit/eb7d8850f4f99e4064b4cd7dda82920d7f07d603) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 04/05/19
- *Committed:* Delete CM1.zip (https://github.com/WagicProject/wagic/commit/934b9d2856889f6d0340d46ccb9f511e0d966c91) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Commander's Arsenal (CM1) had the wrong name, this is the correct name and set code. (https://github.com/WagicProject/wagic/commit/0af443b7537d57bfa49febc871ab47d8c0949869) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez)) 

### 02/05/19
- *Committed:* War of the Spark _cards.dat (https://github.com/WagicProject/wagic/commit/41e160ae3430f56a1d5d9ee589c9abd38646307d) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez)) 

- *Committed:* Major Corrections to WAR and removing crashing cards (https://github.com/WagicProject/wagic/commit/8b620627d9941c89b30bf263eafaa91c48ee9c21) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Double face cards should be marked as tokens (https://github.com/WagicProject/wagic/commit/d943778309ad298fe6f4356574967ae51a9ef83c) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Teaching AI proliferate and shocklands. Some minor changes, corrections to XLN and C17, changing name of game mode from horde to tribal wars (https://github.com/WagicProject/wagic/commit/06cb835a5bb447704655f75aac00163760885176) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 26/04/19
- *Committed:* Preparing for a release, bug fixes and adjustments to game modes (https://github.com/WagicProject/wagic/commit/f1a529576e5c2f5635e68a1e39a2ab349239f2d5) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* Uploading sets since Amonkhet until War of the spark (https://github.com/WagicProject/wagic/commit/1af3cff419766abf8db37199a9f4148dc3703d76) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 20/04/19
- *Committed:* Set limited game mode, AI changes, primitives and _cards.dat corrections (https://github.com/WagicProject/wagic/commit/5db6b807bad7d998457d042126f46f7298c1ed78) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* New game modes (https://github.com/WagicProject/wagic/commit/1c6b0bdfd56a47506547fe1c5b2bb405a7c25715) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 18/04/19
- *Committed:* Bug fix for the biggest crash and fixes to the primitives (https://github.com/WagicProject/wagic/commit/3dfcc65fa6c7295c635d2652178918ba6d41851f) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 15/04/19
- *Committed:* Fixing primitives and mayor bugs report (https://github.com/WagicProject/wagic/commit/c3937ce517706a129597cdbfdebecc6b2b5ea467) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

- *Committed:* New game modes, bug fixes in primitives, improving AI, new planeswalkers type rule. New rules based on vanguard, 3 new random game modes, one is tribal and one uses any card in the game. Several corrections and bugs fixes. Cards with x in their cost and that can target any player used to crash the game. Teaching AI new things and changing values of efficiency. You can have multiple "Jace" planeswalkers, you can't have two of the exact same name (no two Jace, the mind sculptor). (https://github.com/WagicProject/wagic/commit/ab1fbaa806fc48509b01a1649c7e8dab8745ec7b) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 01/04/19
- *Committed:* Last March Update. Some bug fixing to the primitives, the fetchlands are so buggy that I restored to a simple version. Some minor UI improvements (https://github.com/WagicProject/wagic/commit/6eca5bea2fd19284d0c254ff21e5ec12b3d4018d) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 28/03/19
- *Committed:* - removing debug info after travis build fix (https://github.com/WagicProject/wagic/commit/e4d73d95ecb2d903ffd20b4eb43219ff11599cdc) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build travis script debug pt 7 (groovy, Java 8 incompatibility) (https://github.com/WagicProject/wagic/commit/363383df6d9e2731add17e7916809269602c1f26) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build travis script debug pt 6 (ant error log too long) (https://github.com/WagicProject/wagic/commit/0d05804376e9643fc861a29118be1940133735b6) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build travis script debug pt 5 (https://github.com/WagicProject/wagic/commit/cdd36a404b64ef6dce0c2d269a3867d801d05562) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - fix bad commit travis.yml (https://github.com/WagicProject/wagic/commit/64c509980ca128a2ceabcb336819e04a39e13a2b) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build travis script debug pt 4 (https://github.com/WagicProject/wagic/commit/46ba0182013f8029ad3ab01f919ecb9e765b5929) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build travis script debug pt 3 (https://github.com/WagicProject/wagic/commit/0cb931e6e284f83ca98a0eebb5b0282047dbd356) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build travis script debug pt 2 (https://github.com/WagicProject/wagic/commit/58af6f73d98e0fc04c4b787d4a5c773e4d45bff6) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build travis script debug (https://github.com/WagicProject/wagic/commit/e81f1299cb069aa4e06eb0aeb1cdec403f9dca2f) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build android dependancies pt 3 (https://github.com/WagicProject/wagic/commit/9b5f9684b013e1466d246c8f2642a83a5b4c2a94) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build android dependancies pt 2 (https://github.com/WagicProject/wagic/commit/d934c0948c1bf9cb140fc061baeb56d038e5b5b1) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* - try to fix travis build android dependancies (https://github.com/WagicProject/wagic/commit/917a3a88df7c2c3d3bc2eee909a634b296778cc0) ([Rolzad73](https://github.com/Rolzad73))

### 27/03/19
- *Committed:* - try to fix travis build (https://github.com/WagicProject/wagic/commit/dc5b8a8c523219b04a9a436d0691b377f9faffb7) ([Rolzad73](https://github.com/Rolzad73))

- *Committed:* Uploading a couple of years of sets (https://github.com/WagicProject/wagic/commit/d2b44e652b359824f54cbd3c49221f042b6cc54d) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 26/03/19
- *Committed:* Update primitives (https://github.com/WagicProject/wagic/commit/1ce3b469e7adf2526a404a1fafaf34df1e8ec8c6) ([EduardoMunozGomez](https://github.com/EduardoMunozGomez))

### 03/06/16
- *Committed:* unused variable (https://github.com/WagicProject/wagic/commit/be53c68d28e0cf0ad0f7955c4c87df0d41057d92) ([kevlahnota](https://github.com/kevlahnota))

### 03/06/16
- *Committed:* Cascade Class (https://github.com/WagicProject/wagic/commit/62fa9acc9ff5c924963b25582645c2cb09c8984f) ([kevlahnota](https://github.com/kevlahnota))

### 03/06/16
- *Committed:* Merge remote-tracking branch 'refs/remotes/WagicProject/master' (https://github.com/WagicProject/wagic/commit/0493c4063f0e5cb355064152bb93f492ffa512f2) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Committed:* Should fix Macosx build (https://github.com/WagicProject/wagic/commit/956d0a736d746b6c00febf85fcc21983607b873c) ([Xawotihs](https://github.com/Xawotihs))

### 02/06/16
- *Merged remote-tracking:* Merge remote-tracking branch 'refs/remotes/WagicProject/master' (https://github.com/WagicProject/wagic/commit/b407701d942fafb672029eb5d98fefc0f081bb46) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Merged pull-request:* Merge pull request #672 from kevlahnota/master (https://github.com/WagicProject/wagic/commit/b407701d942fafb672029eb5d98fefc0f081bb46) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Committed:* Cascade (https://github.com/WagicProject/wagic/commit/b407701d942fafb672029eb5d98fefc0f081bb46) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Committed:* Merge remote-tracking branch 'refs/remotes/WagicProject/master' (https://github.com/WagicProject/wagic/commit/b407701d942fafb672029eb5d98fefc0f081bb46) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Merged pull-request:*  Merge pull request #671 from kevlahnota/master (https://github.com/WagicProject/wagic/commit/b407701d942fafb672029eb5d98fefc0f081bb46) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Committed:* parenthesis try to fix psp build (https://github.com/WagicProject/wagic/commit/ed6755496f39c3f4f104f407c504ba6eca03bc28) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Committed:* whitespace (https://github.com/WagicProject/wagic/commit/4530115506fb99382c6728106f0dd689f8058940) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Committed:* Cant life change (https://github.com/WagicProject/wagic/commit/aabb9053131435332fb737243e05e197c58c0547) ([kevlahnota](https://github.com/kevlahnota))

### 02/06/16
- *Merged pull-request:*  Merge pull request #1 from WagicProject/master (https://github.com/WagicProject/wagic/commit/aaf2d271bc6b547edc261424f7049a2499288f0c) ([kevlahnota](https://github.com/kevlahnota))

- *Committed:* added abilities= keyword "devoid" (https://github.com/WagicProject/wagic/commit/1369a08863be19197eea8480a5de3e13daaecf4e) ([zethfoxster](https://github.com/zethfoxster))

### 01/06/16
- *Committed:* minor skip phase (https://github.com/WagicProject/wagic/commit/ac9dfd570a102e31fbb1cdfda2326fe12926fd38) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* Fix crash for Deck Menu (https://github.com/WagicProject/wagic/commit/5cd5f4b38e2c4c121f7e635dbf3db9b4c685cb37) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* Add "Snow" Cards (https://github.com/WagicProject/wagic/commit/57c7796424b92b82c3e4743e11900100eaf4ce51) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* Merge branch 'master' of https://github.com/WagicProject/wagic (https://github.com/WagicProject/wagic/commit/dd6c1c3d01f8a49044d754f3e9251c61f904c6c9) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* tidying up (https://github.com/WagicProject/wagic/commit/fd89d51f843e76f70f3825e4c62206e3eb043829) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* Update test, colorless mana changes (https://github.com/WagicProject/wagic/commit/712fc26d56e38da6e12804b0d0a22262f65060ac) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* re add the comparison (https://github.com/WagicProject/wagic/commit/9e501877407ecabbb38776f7b340b2a54545b30f) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* try to fix psp compilation (https://github.com/WagicProject/wagic/commit/a36b47e5003480393fa5dca649634862aa2a6008) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* Snow Mana produced by snow permanent (https://github.com/WagicProject/wagic/commit/a454c3e5646f8ae08b494aefe7bee277b823ec98) ([kevlahnota](https://github.com/kevlahnota))

### 01/06/16
- *Committed:* ok sorry about the confusion, so the issue was with {1}{c} and how its handled. we only need to replace the adding of colorless in the manapool, the old colorless is still very very valid and should be left in place as is. (https://github.com/WagicProject/wagic/commit/26b30cb20d22c73dabb159d76368032c3d13d4ac) ([zethfoxster](https://github.com/zethfoxster))

### 30/05/16
- *Committed:* whoops heres the other half. tested with following arbor elf test card (https://github.com/WagicProject/wagic/commit/7639e18c39c7bf5ede02146e866309c86a01b2af) ([zethfoxster](https://github.com/zethfoxster))

### 30/05/16
- *Committed:* defining "colorless" waste land mana. (https://github.com/WagicProject/wagic/commit/80d0ee4dc312a2af0609f6dff21f6bd4d9a65290) ([zethfoxster](https://github.com/zethfoxster))

### 30/05/16
- *Committed:* deckstats crash when "unlock cards" is used [#\668](https://github.com/WagicProject/wagic/commit/8a6b4a49f5900be63ff0add8744cc27396d3cbf4) ([zethfoxster](https://github.com/zethfoxster))

### 29/05/16
- *Committed:* super rare loop in ai combos and fix [#\667](https://github.com/WagicProject/wagic/commit/00adb20dc71fff4c46f3420c240960ed39c78588) ([kevlahnota](https://github.com/kevlahnota))

### 29/05/16
- *Committed:* add "colorless" manacost (https://github.com/WagicProject/wagic/commit/22d4cbbd1ece8f88a866055f1113dbc954cc1eae) ([kevlahnota](https://github.com/kevlahnota))

### 29/05/16
- *Committed:* count valid targets any creature (https://github.com/WagicProject/wagic/commit/1d81c12150ef221b29635425f6d2388abea3d0ef) ([kevlahnota](https://github.com/kevlahnota))

### 29/05/16
- *Committed:* count valid targets, count creature/s that are valid target/s (https://github.com/WagicProject/wagic/commit/0fed1ba1abaf73f22dd730a48c011ab3f18ec047) ([kevlahnota](https://github.com/kevlahnota))

### 29/05/16
- *Committed:* Bug Fix, Fix send to library as a cost and add support for moving a card to graveyard as a cost(cards like Void Attendant from Battle for Zendikar) (https://github.com/WagicProject/wagic/commit/0fed1ba1abaf73f22dd730a48c011ab3f18ec047) ([kevlahnota](https://github.com/kevlahnota))

### 29/05/16
- *Committed:* fix type Ajani Steadfast forcefield for play and planeswalker only.. (https://github.com/WagicProject/wagic/commit/0fed1ba1abaf73f22dd730a48c011ab3f18ec047) ([kevlahnota](https://github.com/kevlahnota))

### 29/05/16
- *Committed:* ajani steadfast ajani steadfast emblem (https://github.com/WagicProject/wagic/commit/0fed1ba1abaf73f22dd730a48c011ab3f18ec047) ([kevlahnota](https://github.com/kevlahnota))

### 29/05/16
- *Committed:* Indicator Green border indicator for pay zero, can play from exile or can play from graveyard cards... (https://github.com/WagicProject/wagic/commit/0fed1ba1abaf73f22dd730a48c011ab3f18ec047) ([kevlahnota](https://github.com/kevlahnota))

### 29/05/16
- *Committed:* Ral Zarek Fix Loyalty Counter (https://github.com/WagicProject/wagic/commit/e45290e834cbda242b5393a72b7640d9d0edd902) ([kevlahnota](https://github.com/kevlahnota))

### 28/05/16
- *Committed:* Reversed android and Qt apt stuff (https://github.com/WagicProject/wagic/commit/95cec8874232c9f4df3b60d3da71b495ba67679b) ([Xawotihs](https://github.com/Xawotihs))

### 28/05/16
- *Committed:* Moved -qq options (https://github.com/WagicProject/wagic/commit/95cec8874232c9f4df3b60d3da71b495ba67679b) ([Xawotihs](https://github.com/Xawotihs))

### 28/05/16
- *Committed:* Removed comment (https://github.com/WagicProject/wagic/commit/95cec8874232c9f4df3b60d3da71b495ba67679b) ([Xawotihs](https://github.com/Xawotihs))

### 28/05/16
- *Committed:* Replaced Qt ppa (https://github.com/WagicProject/wagic/commit/95cec8874232c9f4df3b60d3da71b495ba67679b) ([Xawotihs](https://github.com/Xawotihs))

### 28/05/16
- *Committed:* Fix summoning sickness Cards like control magic (using alias=1194) always resets the summoning sickness, commenting this solves the problem...  (https://github.com/WagicProject/wagic/commit/18430dca25ff38c39455cc671a3a805d70a5d794) ([kevlahnota](https://github.com/kevlahnota))

### 27/05/16
- *Committed:* Update appveyor.yml try to fix appveyor (https://github.com/WagicProject/wagic/commit/8a374f53bc24781b390cb4b2a9dd75210307726a) ([kevlahnota](https://github.com/kevlahnota))

#### 26/5/16
- *Committed:* Nasty memleak crashing devices and its fix. #664 thanks zethfox  [#\f5d00dd] (https://github.com/WagicProject/wagic/commit/7a34543004e310342faade11befc4fac51715685) ([kevlahnota](https://github.com/kevlahnota))

#### 8/3/16
- *Merged pull-request:* andAbility on ATokenCreator, Extend AAFlip [#\824] (https://github.com/WagicProject/wagic/pull/824) ([kevlahnota](https://github.com/kevlahnota))

#### 7/31/16
- *Merged pull-request:* Fix Chandra, Flamecaller & Cryptolith Rite [#\818] (https://github.com/WagicProject/wagic/pull/818) ([kevlahnota](https://github.com/kevlahnota))

#### 7/28/16
- *Merged pull-request:* changing some of the logic to the previous fixes, we want to avoid using code that looks for specific card names. [#\800] (https://github.com/WagicProject/wagic/pull/800) ([zethfoxster](https://github.com/zethfoxster))

#### 7/26/16
- *Merged pull-request:* granted flashback [#\791] (https://github.com/WagicProject/wagic/pull/791) ([kevlahnota](https://github.com/kevlahnota))

#### 7/23/16
- *Merged pull-request:* Fix Flying vs Moat (multiples of them). Fixes issue #526 [#\783] (https://github.com/WagicProject/wagic/pull/783) ([kevlahnota](https://github.com/kevlahnota))

#### 7/19/16
- *Merged pull-request:* pushing to master [#\770] (https://github.com/WagicProject/wagic/pull/770) ([zethfoxster](https://github.com/zethfoxster))

#### 7/18/16
- *Merged pull-request:* Fix Deck Menu layer, Change Main Menu Layout, Fix Crash cdaactive on tokens [#\765] (https://github.com/WagicProject/wagic/pull/765) ([kevlahnota](https://github.com/kevlahnota))

#### 7/12/16
- *Merged pull-request:* Pushing fixes [#\750] (https://github.com/WagicProject/wagic/pull/750) ([zethfoxster](https://github.com/zethfoxster))

- *Merged pull-request:* try to fix failed logic here [#\747] (https://github.com/WagicProject/wagic/pull/747) ([kevlahnota](https://github.com/kevlahnota))

#### 7/9/16
- *Merged pull-request:* pushing to master [#\738] (https://github.com/WagicProject/wagic/pull/738) ([zethfoxster](https://github.com/zethfoxster))

- *Merged pull-request:* Sorted Primitives and Cleanup Tabs [#\736] (https://github.com/WagicProject/wagic/pull/736) ([kevlahnota](https://github.com/kevlahnota))

#### 7/7/16
- *Merged pull-request:* Sorted Primitives and Cleanup Tabs [#\727] (https://github.com/WagicProject/wagic/pull/719) ([kevlahnota](https://github.com/kevlahnota))

- *Merged pull-request:* push to wagicproject [#\725] (https://github.com/WagicProject/wagic/pull/725) ([zethfoxster](https://github.com/zethfoxster))

#### 7/4/16
- *Merged pull-request:* Sorted Primitives & Updated Premium Deck Series [#\719] (https://github.com/WagicProject/wagic/pull/719) ([kevlahnota](https://github.com/kevlahnota))

- *Merged pull-request:* Fails but I have no idea why [#\717] (https://github.com/WagicProject/wagic/pull/717) ([zethfoxster](https://github.com/zethfoxster))

- *Merged pull-request:* Updated Sets [#\715] (https://github.com/WagicProject/wagic/pull/715) ([kevlahnota](https://github.com/kevlahnota))

#### 7/2/16
- *Merged pull-request:* Updated Masters, Beginners, Duels and Vault Sets [#\714] (https://github.com/WagicProject/wagic/pull/714) ([kevlahnota](https://github.com/kevlahnota))

#### 7/1/16
- *Merged pull-request:* Cleaned and Sorted all Core and Expansion Sets [#\713] (https://github.com/WagicProject/wagic/pull/713) ([kevlahnota](https://github.com/kevlahnota))

- *Merged pull-request:* Soulbond Pair Indicator [#\712] (https://github.com/WagicProject/wagic/pull/712) ([kevlahnota](https://github.com/kevlahnota))

#### 6/30/16
- *Merged pull-request:* Sorted Primitives [#\711] (https://github.com/WagicProject/wagic/pull/711) ([kevlahnota](https://github.com/kevlahnota))

- *Merged pull-request:* Add Shackle Ability [#\710] (https://github.com/WagicProject/wagic/pull/710) ([kevlahnota](https://github.com/kevlahnota))

- *Merged pull-request:* Fix Cascade, Crash fix on lastController and some corrections [#\709] (https://github.com/WagicProject/wagic/pull/709) ([kevlahnota](https://github.com/kevlahnota))

#### 6/29/16
- *Merged pull-request:* Updated Primitives and Hide Highlight Border [#\707] (https://github.com/WagicProject/wagic/pull/707) ([kevlahnota](https://github.com/kevlahnota))

- *Committed:* Pretty huge patch here(sorry old habits never die :( )  [#\6ee00c1] (https://github.com/WagicProject/wagic/commit/6ee00c138ce374d54cb3ee034575ce440288ca0e) ([zethfoxster](https://github.com/zethfoxster))

#### 6/23/16
- *Merged pull-request:* Fix producecolor:color [#\704] (https://github.com/WagicProject/wagic/pull/704) ([kevlahnota](https://github.com/kevlahnota))

#### 6/17/16
- *Merged pull-request:* Force Mounting 2 [#\699] (https://github.com/WagicProject/wagic/pull/699) ([kevlahnota](https://github.com/kevlahnota))

#### 6/16/16
- *Merged pull-request:* Import Deck Options [#\697] (https://github.com/WagicProject/wagic/pull/697) ([kevlahnota](https://github.com/kevlahnota))

#### 6/11/16
- *Merged pull-request:* Refactor & Devotion [#\690] (https://github.com/WagicProject/wagic/pull/690) ([kevlahnota](https://github.com/kevlahnota))

#### 6/9/16
- *Closed issue:* Clone goes to graveyard before being able to copy anything [\#546](https://github.com/WagicProject/wagic/issues/546)

- *Closed issue:* Blinking an aura causes freeze [\#541](https://github.com/WagicProject/wagic/issues/541)

- *Closed issue:* Profile name at upper left side is not being updated correctly [\#469](https://github.com/WagicProject/wagic/issues/469)

- *Closed issue:* Giving card C protection from X does not remove cards attached to C which have quality X [\#464](https://github.com/WagicProject/wagic/issues/464)

- *Closed issue:* (graphical glitch) wood / gold textures [\#461](https://github.com/WagicProject/wagic/issues/461)

- *Closed issue:* Clone does not get all P/T bonuses [\#448](https://github.com/WagicProject/wagic/issues/448)

- *Closed issue:* Card "threaten" messes up the GUI [\#473](https://github.com/WagicProject/wagic/issues/473)

#### 6/7/16
- *Closed issue:* Emrakul isn't killed by creatures with deathtouch [\#597](https://github.com/WagicProject/wagic/issues/597)

#### 6/3/16
- *Merged pull-request:* Cost Increaser & Reducer Fix [#\676] (https://github.com/WagicProject/wagic/pull/676) ([kevlahnota](https://github.com/kevlahnota))

#### 5/30/16
- *Fixed Bug:* deckstats crash when "unlock cards" is used [#\668] (https://github.com/WagicProject/wagic/issues/668) ([zethfoxster](https://github.com/zethfoxster))

#### 5/26/16
- *Closed issue:* Nasty memleak crashing devices and its fix. [#\664] (https://github.com/WagicProject/wagic/issues/664) ([zethfoxster](https://github.com/zethfoxster))

#### 11/08/15
- *Merged pull-request:* produce mana for lands you/opponent could produce [#\658] (https://github.com/WagicProject/wagic/pull/658) ([kevlahnota](https://github.com/kevlahnota))

#### 11/07/15
- *Merged pull-request:* Manacost Changes, Anyzone for CDA, PayZero Cost [#\656] (https://github.com/WagicProject/wagic/pull/656) ([kevlahnota](https://github.com/kevlahnota))

#### 10/31/15
- *Merged pull-request:* Exile Zone, Altercost and Hand modifier [#\653] (https://github.com/WagicProject/wagic/pull/653) ([kevlahnota](https://github.com/kevlahnota))

#### 10/17/15
- *Merged pull-request:* Fix issue #473 #784 [#\646] (https://github.com/WagicProject/wagic/pull/646) ([kevlahnota](https://github.com/kevlahnota))

#### 10/15/15
- *Merged pull-request:* Bug Fix: Planeswalker Rule & ABlink return to play ability for Aura cards [#\644] (https://github.com/WagicProject/wagic/pull/644) ([kevlahnota](https://github.com/kevlahnota))

#### 10/14/15
- *Merged pull-request:* Fix Legend Rule [#\643] (https://github.com/WagicProject/wagic/pull/643) ([kevlahnota](https://github.com/kevlahnota))

#### 10/12/15
- *Merged pull-request:* Fix crash bug, support doubled res texture for background, avatars, menutitle [#\641] (https://github.com/WagicProject/wagic/pull/641) ([kevlahnota](https://github.com/kevlahnota))

#### 10/02/15
- *Merged pull-request:* LKI for power, toughness and basic abilities, produceextra ability [#\636] (https://github.com/WagicProject/wagic/pull/636) ([kevlahnota](https://github.com/kevlahnota))

#### 09/29/15
- *Merged pull-request:* Fix Recover Cards, Cloner, Copier, and Preliminary support for Madness [#\635] (https://github.com/WagicProject/wagic/pull/635) ([kevlahnota](https://github.com/kevlahnota))

#### 09/22/15
- *Merged pull-request:* Auraward, unattach event and statebased action for protection from quality [#\631] (https://github.com/WagicProject/wagic/pull/631) ([kevlahnota](https://github.com/kevlahnota))

#### 09/19/15
- *Merged pull-request:* PT Switch like Layer 7e and Token Indicator [#\626] (https://github.com/WagicProject/wagic/pull/626) ([kevlahnota](https://github.com/kevlahnota))

#### 09/18/15
- *Merged pull-request:* Token Cloning fix and colored PT [#\624] (https://github.com/WagicProject/wagic/pull/624) ([kevlahnota](https://github.com/kevlahnota))

#### 09/15/15
- *Merged pull-request:* CDA and X manacost on stack [\#623] (https://github.com/WagicProject/wagic/pull/623) ([kevlahnota](https://github.com/kevlahnota))

#### 08/12/14
- *Merged pull-request:* Fix for guild_keywords Devotion, added some "Chroma" cards [\#606](https://github.com/WagicProject/wagic/pull/606) ([kevlahnota](https://github.com/kevlahnota))

- *Merged pull-request:* added guild keywords, added specific life cost [\#605](https://github.com/WagicProject/wagic/pull/605) ([kevlahnota](https://github.com/kevlahnota))

- *Merged pull-request:* Buyback issue and a fix [\#604](https://github.com/WagicProject/wagic/pull/604) ([apollovy](https://github.com/apollovy))

- *Merged pull-request:* Feature/ios update xcode5.1 [\#602](https://github.com/WagicProject/wagic/pull/602) ([mjnguyen](https://github.com/mjnguyen))

- *Merged pull-request:* Remove dead code and fix circular initialization [\#601](https://github.com/WagicProject/wagic/pull/601) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* update android build tools version [\#600](https://github.com/WagicProject/wagic/pull/600) ([Rolzad73](https://github.com/Rolzad73))

- *Merged pull-request:* Attempt to make android tools fetching for TravisCI explicit and futureproof [\#599](https://github.com/WagicProject/wagic/pull/599) ([Rolzad73](https://github.com/Rolzad73))

- *Merged pull-request:* Damager keyword [\#598](https://github.com/WagicProject/wagic/pull/598) ([bjornsnoen](https://github.com/bjornsnoen))

- *Merged pull-request:* Fix a rarity mistake and a grammatical error [\#596](https://github.com/WagicProject/wagic/pull/596) ([bjornsnoen](https://github.com/bjornsnoen))

- *Merged pull-request:* Fix bug where the phase wheel got out of sync [\#588](https://github.com/WagicProject/wagic/pull/588) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* More easing [\#586](https://github.com/WagicProject/wagic/pull/586) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Added Avatar Tokens created by Ajani Goldmane. [\#585](https://github.com/WagicProject/wagic/pull/585) ([bjornsnoen](https://github.com/bjornsnoen))

- *Merged pull-request:* Cleanup, usability fixes and source code documentation for DeckView.h and GridDeckView.h [\#583](https://github.com/WagicProject/wagic/pull/583) ([ZobyTwo](https://github.com/ZobyTwo))

- *Fixed bug:* Blight does not destroy at the end of turn but right now [\#592](https://github.com/WagicProject/wagic/issues/592)

- *Fixed bug:* game freezes on Android when the phone returns from "sleep mode" [\#544](https://github.com/WagicProject/wagic/issues/544)

- *Fixed bug:* Android port needs to be able to respond to attaching/detaching devices to it [\#522](https://github.com/WagicProject/wagic/issues/522)

## [alpha-195] (https://github.com/WagicProject/wagic/tree/alpha-195)
#### 07/12/13
- *Merged pull-request:* Reset positions and filters when reopening the editor [\#578](https://github.com/WagicProject/wagic/pull/578) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Deck viewer [\#577](https://github.com/WagicProject/wagic/pull/577) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Do not specify the system directory in JGE [\#576](https://github.com/WagicProject/wagic/pull/576) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Changed Travis build file to use Android API 10 and Android build tools 18.1.1 [\#575](https://github.com/WagicProject/wagic/pull/575) ([Rolzad73](https://github.com/Rolzad73))

- *Merged pull-request:* Feature/play from grave [\#574](https://github.com/WagicProject/wagic/pull/574) ([pankdm](https://github.com/pankdm))

- *Merged pull-request:* Fix Valgrind warnings appearing during the test suit. [\#573](https://github.com/WagicProject/wagic/pull/573) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Strings 2 [\#572](https://github.com/WagicProject/wagic/pull/572) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Android travis ci [\#570](https://github.com/WagicProject/wagic/pull/570) ([xawotihs](https://github.com/xawotihs))

- *Merged pull-request:* Android NDK build fix [\#569](https://github.com/WagicProject/wagic/pull/569) ([Rolzad73](https://github.com/Rolzad73))

- *Merged pull-request:* Add a few namespaces and fix header guards. [\#564](https://github.com/WagicProject/wagic/pull/564) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Updated deckmenu.cpp to properly render deck description and summary [\#562](https://github.com/WagicProject/wagic/pull/562) ([citiral](https://github.com/citiral))

- *Merged pull-request:* Fix some valgrind memcheck warnings. However there remains one... [\#561](https://github.com/WagicProject/wagic/pull/561) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Remove some dead code [\#560](https://github.com/WagicProject/wagic/pull/560) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Show output only for failing tests [\#559](https://github.com/WagicProject/wagic/pull/559) ([pankdm](https://github.com/pankdm))

- *Merged pull-request:* Defines/Typos [\#557](https://github.com/WagicProject/wagic/pull/557) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Simplify Damage::resolve a bit [\#556](https://github.com/WagicProject/wagic/pull/556) ([ZobyTwo](https://github.com/ZobyTwo))

- *Merged pull-request:* Fix cranial plating [\#555](https://github.com/WagicProject/wagic/pull/555) ([pankdm](https://github.com/pankdm))

- *Merged pull-request:* Android cleanup [\#1](https://github.com/WagicProject/wagic/pull/1) ([Rolzad73](https://github.com/Rolzad73))

- *Fixed bug:* Iona and Nin don't work [\#527](https://github.com/WagicProject/wagic/issues/527)

- *Closed issue:* Less verbose output of tests at Travis [\#558](https://github.com/WagicProject/wagic/issues/558)

## [wagic-0.19.2] (https://github.com/WagicProject/wagic/tree/wagic-v0.19.2)
#### 28/10/13
