#include "LanguageDefines.h"

#ifdef POLISH

/*

******************************************************************************************************
**                                  IMPORTANT TRANSLATION NOTES **
******************************************************************************************************

GENERAL INSTRUCTIONS
- Always be aware that foreign strings should be of equal or shorter length than the English
equivalent. I know that this is difficult to do on many occasions due to the nature of foreign
languages when compared to English.  By doing so, this will greatly reduce the amount of work on
both sides.  In most cases (but not all), JA2 interfaces were designed with just enough space to fit
the English word. The general rule is if the string is very short (less than 10 characters), then
it's short because of interface limitations.  On the other hand, full sentences commonly have little
limitations for length. Strings in between are a little dicey.
- Never translate a string to appear on multiple lines.  All strings L"This is a really long
string...", must fit on a single line no matter how long the string is.  All strings start with L"
and end with ",
- Never remove any extra spaces in strings.  In addition, all strings containing multiple sentences
only have one space after a period, which is different than standard typing convention.  Never
modify sections of strings contain combinations of % characters.  These are special format
characters and are always used in conjunction with other characters.  For example, %s means string,
and is commonly used for names, locations, items, etc.  %d is used for numbers.  %c%d is a character
and a number (such as A9).
        %% is how a single % character is built.  There are countless types, but strings containing
these special characters are usually commented to explain what they mean.  If it isn't commented,
then if you can't figure out the context, then feel free to ask SirTech.
- Comments are always started with // Anything following these two characters on the same line are
        considered to be comments.  Do not translate comments.  Comments are always applied to the
following string(s) on the next line(s), unless the comment is on the same line as a string.
- All new comments made by SirTech will use "//@@@ comment" (without the quotes) notation.  By
searching for @@@ everytime you recieve a new version, it will simplify your task and identify
special instructions. Commonly, these types of comments will be used to ask you to abbreviate a
string.  Please leave the comments intact, and SirTech will remove them once the translation for
that particular area is resolved.
- If you have a problem or question with translating certain strings, please use "//!!! comment"
        (without the quotes).  The syntax is important, and should be identical to the comments used
with @@@ symbols.  SirTech will search for !!! to look for your problems and questions.  This is a
more efficient method than detailing questions in email, so try to do this whenever possible.



FAST HELP TEXT -- Explains how the syntax of fast help text works.
**************

1) BOLDED LETTERS
        The popup help text system supports special characters to specify the hot key(s) for a
button. Anytime you see a '|' symbol within the help text string, that means the following key is
assigned to activate the action which is usually a button.

        EX:  L"|Map Screen"

        This means the 'M' is the hotkey.  In the game, when somebody hits the 'M' key, it activates
that button.  When translating the text to another language, it is best to attempt to choose a word
that uses 'M'.  If you can't always find a match, then the best thing to do is append the 'M' at the
end of the string in this format:

        EX:  L"Ecran De Carte (|M)"  (this is the French translation)

        Other examples are used multiple times, like the Esc key  or "|E|s|c" or Space ->
(|S|p|a|c|j|a)

2) NEWLINE
  Any place you see a \n within the string, you are looking at another string that is part of the
fast help text system.  \n notation doesn't need to be precisely placed within that string, but
whereever you wish to start a new line.

        EX:  L"Clears all the mercs' positions,\nand allows you to re-enter them manually."

        Would appear as:

                                Clears all the mercs' positions,
                                and allows you to re-enter them manually.

        NOTE:  It is important that you don't pad the characters adjacent to the \n with spaces.  If
we did this in the above example, we would see

        WRONG WAY -- spaces before and after the \n
        EX:  L"Clears all the mercs' positions, \n and allows you to re-enter them manually."

        Would appear as: (the second line is moved in a character)

                                Clears all the mercs' positions,
                                 and allows you to re-enter them manually.


@@@ NOTATION
************

        Throughout the text files, you'll find an assortment of comments.  Comments are used to
describe the text to make translation easier, but comments don't need to be translated.  A good
thing is to search for
        "@@@" after receiving new version of the text file, and address the special notes in this
manner.

!!! NOTATION
************

        As described above, the "!!!" notation should be used by you to ask questions and address
problems as SirTech uses the "@@@" notation.

*/

uint16_t ItemNames[MAXITEMS][80] = {
    L"",
};

uint16_t ShortItemNames[MAXITEMS][80] = {
    L"",
};

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
uint16_t AmmoCaliber[][20] = {
    L"0",         L".38 cal", L"9mm",    L".45 cal",  L".357 cal",
    L"12 gauge",  L"CAWS",    L"5.45mm", L"5.56mm",   L"7.62mm NATO",
    L"7.62mm WP", L"4.7mm",   L"5.7mm",  L"Monstrum", L"Rakiety",
    L"",  // dart
    L"",  // flame
};

// This BobbyRayAmmoCaliber is virtually the same as AmmoCaliber however the bobby version doesnt
// have as much room for the words.
//
// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
uint16_t BobbyRayAmmoCaliber[][20] = {
    L"0",         L".38 cal", L"9mm",    L".45 cal",  L".357 cal",
    L"12 gauge",  L"CAWS",    L"5.45mm", L"5.56mm",   L"7.62mm N.",
    L"7.62mm WP", L"4.7mm",   L"5.7mm",  L"Monstrum", L"Rakiety",
    L"",  // dart
};

uint16_t WeaponType[][30] = {
    L"Inny",    L"Pistolet",           L"Pistolet maszynowy", L"Karabin maszynowy",
    L"Karabin", L"Karabin snajperski", L"Karabin bojowy",     L"Lekki karabin maszynowy",
    L"Strzelba"};

uint16_t TeamTurnString[][STRING_LENGTH] = {
    L"Tura gracza",  // player's turn
    L"Tura przeciwnika", L"Tura stworzeń", L"Tura samoobrony", L"Tura cywili"
    // planning turn
};

uint16_t Message[][STRING_LENGTH] = {
    L"",

    // In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

    L"%s dostał(a) w głowę i traci 1 punkt inteligencji!",
    L"%s dostał(a) w ramię i traci 1 punkt zręczności!",
    L"%s dostał(a) w klatkę piersiową i traci 1 punkt siły!",
    L"%s dostał(a) w nogi i traci 1 punkt zwinności!",
    L"%s dostał(a) w głowę i traci %d pkt. inteligencji!",
    L"%s dostał(a) w ramię i traci %d pkt. zręczności!",
    L"%s dostał(a) w klatkę piersiową i traci %d pkt. siły!",
    L"%s dostał(a) w nogi i traci %d pkt. zwinności!",
    L"Przerwanie!",

    // The first %s is a merc's name, the second is a string from pNoiseVolStr,
    // the third is a string from pNoiseTypeStr, and the last is a string from pDirectionStr

    L"",  // OBSOLETE
    L"Dotarły twoje posiłki!",

    // In the following four lines, all %s's are merc names

    L"%s przeładowuje.",
    L"%s posiada za mało Punktów Akcji!",
    L"%s udziela pierwszej pomocy. (Naciśnij dowolny klawisz aby przerwać.)",
    L"%s i %s udzielają pierwszej pomocy. (Naciśnij dowolny klawisz aby przerwać.)",
    // the following 17 strings are used to create lists of gun advantages and disadvantages
    // (separated by commas)
    L"niezawodna",
    L"zawodna",
    L"łatwa w naprawie",
    L"trudna do naprawy",
    L"solidna",
    L"niesolidna",
    L"szybkostrzelna",
    L"wolno strzelająca",
    L"daleki zasięg",
    L"krótki zasięg",
    L"mała waga",
    L"duża waga",
    L"niewielkie rozmiary",
    L"szybki ciągły ogień",
    L"brak ciągłego ognia",
    L"duży magazynek",
    L"mały magazynek",

    // In the following two lines, all %s's are merc names

    L"%s: kamuflaż się starł.",
    L"%s: kamuflaż się zmył.",

    // The first %s is a merc name and the second %s is an item name

    L"Brak amunicji w dodatkowej broni!",
    L"%s ukradł(a): %s.",

    // The %s is a merc name

    L"%s ma broń bez funkcji ciągłego ognia.",

    L"Już masz coś takiego dołączone.",
    L"Połączyć przedmioty?",

    // Both %s's are item names

    L"%s i %s nie pasują do siebie.",

    L"Brak",
    L"Wyjmij amunicję",
    L"Dodatki",

    // You cannot use "item(s)" and your "other item" at the same time.
    // Ex:  You cannot use sun goggles and you gas mask at the same time.
    L" %s i %s nie mogą być używane jednocześnie.",

    L"Element, który masz na kursorze myszy może być dołączony do pewnych przedmiotów, poprzez "
    L"umieszczenie go w jednym z czterech slotów.",
    L"Element, który masz na kursorze myszy może być dołączony do pewnych przedmiotów, poprzez "
    L"umieszczenie go w jednym z czterech slotów. (Jednak w tym przypadku, przedmioty do siebie "
    L"nie pasują.)",
    L"Ten sektor nie został oczyszczony z wrogów!",
    L"Wciąż musisz dać %s %s",
    L"%s dostał(a) w głowę!",
    L"Przerwać walkę?",
    L"Ta zmiana będzie trwała. Kontynuować?",
    L"%s ma więcej energii!",
    L"%s poślizgnął(nęła) się na kulkach!",
    L"%s nie chwycił(a) - %s!",
    L"%s naprawił(a) %s",
    L"Przerwanie dla: ",
    L"Poddać się?",
    L"Ta osoba nie chce twojej pomocy.",
    L"NIE SĄDZĘ!",
    L"Aby podróżować helikopterem Skyridera, musisz najpierw zmienić przydział najemników na "
    L"POJAZD/HELIKOPTER.",
    L"%s miał(a) czas by przeładować tylko jedną broń",
    L"Tura dzikich kotów",
};

// the names of the towns in the game

wchar_t* pTownNames[] = {
    L"",         L"Omerta", L"Drassen", L"Alma",   L"Grumm",  L"Tixa",     L"Cambria",
    L"San Mona", L"Estoni", L"Orta",    L"Balime", L"Meduna", L"Chitzena",
};

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per
// second, etc. min is an abbreviation for minutes

wchar_t* sTimeStrings[] = {
    L"Pauza",   L"Normalna", L"5 min.", L"30 min.", L"60 min.",
    L"6 godz.",  // NEW
};

// Assignment Strings: what assignment does the merc  have right now? For example, are they on a
// squad, training, administering medical aid (doctor) or training a town. All are abbreviated. 8
// letters is the longest it can be.

wchar_t* pAssignmentStrings[] = {
    L"Oddz. 1", L"Oddz. 2", L"Oddz. 3", L"Oddz. 4", L"Oddz. 5", L"Oddz. 6", L"Oddz. 7", L"Oddz. 8",
    L"Oddz. 9", L"Oddz. 10", L"Oddz. 11", L"Oddz. 12", L"Oddz. 13", L"Oddz. 14", L"Oddz. 15",
    L"Oddz. 16", L"Oddz. 17", L"Oddz. 18", L"Oddz. 19", L"Oddz. 20",
    L"Służba",    // on active duty
    L"Lekarz",    // administering medical aid
    L"Pacjent",   // getting medical aid
    L"Pojazd",    // sitting around resting
    L"Podróż",    // in transit - abbreviated form
    L"Naprawa",   // repairing
    L"Praktyka",  // training themselves  // ***************NEW******************** as of June 24.
                  // 1998
    L"Samoobr.",  // training a town to revolt // *************NEW******************** as of June
                  // 24, 1998
    L"Instruk.",  // training a teammate
    L"Uczeń",  // being trained by someone else // *******************NEW************** as of June
               // 24, 1998
    L"Nie żyje",  // dead
    L"Obezwł.",   // abbreviation for incapacitated
    L"Jeniec",    // Prisoner of war - captured
    L"Szpital",   // patient in a hospital
    L"Pusty",     // Vehicle is empty
};

wchar_t* pMilitiaString[] = {
    L"Samoobrona",      // the title of the militia box
    L"Bez przydziału",  // the number of unassigned militia troops
    L"Nie możesz przemieszczać oddziałów samoobrony gdy nieprzyjaciel jest w sektorze!",
};

wchar_t* pMilitiaButtonString[] = {
    L"Auto",  // auto place the militia troops for the player
    L"OK",    // done placing militia troops
};

wchar_t* pConditionStrings[] = {
    L"Doskonały",   // the state of a soldier .. excellent health
    L"Dobry",       // good health
    L"Dość dobry",  // fair health
    L"Ranny",       // wounded health
    L"Zmęczony",    // L"Wyczerpany", // tired
    L"Krwawi",      // bleeding to death
    L"Nieprzyt.",   // knocked out
    L"Umierający",  // near death
    L"Nie żyje",    // dead
};

wchar_t* pEpcMenuStrings[] = {
    L"Służba",   // set merc on active duty
    L"Pacjent",  // set as a patient to receive medical aid
    L"Pojazd",   // tell merc to enter vehicle
    L"Wypuść",   // let the escorted character go off on their own
    L"Anuluj",   // close this menu
};

// look at pAssignmentString above for comments

wchar_t* pPersonnelAssignmentStrings[] = {
    L"Oddz. 1",    L"Oddz. 2",  L"Oddz. 3",  L"Oddz. 4",
    L"Oddz. 5",    L"Oddz. 6",  L"Oddz. 7",  L"Oddz. 8",
    L"Oddz. 9",    L"Oddz. 10", L"Oddz. 11", L"Oddz. 12",
    L"Oddz. 13",   L"Oddz. 14", L"Oddz. 15", L"Oddz. 16",
    L"Oddz. 17",   L"Oddz. 18", L"Oddz. 19", L"Oddz. 20",
    L"Służba",     L"Lekarz",   L"Pacjent",  L"Pojazd",
    L"Podróż",     L"Naprawa",  L"Praktyka", L"Trenuje samoobronę",
    L"Instruktor", L"Uczeń",    L"Nie żyje", L"Obezwładniony",
    L"Jeniec",     L"Szpital",
    L"Pusty",  // Vehicle is empty
};

// refer to above for comments

wchar_t* pLongAssignmentStrings[] = {
    L"Oddział 1",      L"Oddział 2",  L"Oddział 3",  L"Oddział 4",
    L"Oddział 5",      L"Oddział 6",  L"Oddział 7",  L"Oddział 8",
    L"Oddział 9",      L"Oddział 10", L"Oddział 11", L"Oddział 12",
    L"Oddział 13",     L"Oddział 14", L"Oddział 15", L"Oddział 16",
    L"Oddział 17",     L"Oddział 18", L"Oddział 19", L"Oddział 20",
    L"Służba",         L"Lekarz",     L"Pacjent",    L"Pojazd",
    L"W podróży",      L"Naprawa",    L"Praktyka",   L"Trenuj samoobronę",
    L"Trenuj oddział", L"Uczeń",      L"Nie żyje",   L"Obezwładniony",
    L"Jeniec",         L"W szpitalu",
    L"Pusty",  // Vehicle is empty
};

// the contract options

wchar_t* pContractStrings[] = {
    L"Opcje kontraktu:",
    L"",                       // a blank line, required
    L"Zaproponuj 1 dzień",     // offer merc a one day contract extension
    L"Zaproponuj 1 tydzień",   // 1 week
    L"Zaproponuj 2 tygodnie",  // 2 week
    L"Zwolnij",                // end merc's contract
    L"Anuluj",                 // stop showing this menu
};

wchar_t* pPOWStrings[] = {
    L"Jeniec",  // an acronym for Prisoner of War
    L"??",
};

wchar_t* pLongAttributeStrings[] = {
    L"SIŁA",  // The merc's strength attribute. Others below represent the other attributes.
    L"ZRĘCZNOŚĆ",
    L"ZWINNOŚĆ",
    L"INTELIGENCJA",
    L"UMIEJĘTNOŚCI STRZELECKIE",
    L"WIEDZA MEDYCZNA",
    L"ZNAJOMOŚĆ MECHANIKI",
    L"UMIEJĘTNOŚĆ DOWODZENIA",
    L"ZNAJOMOŚĆ MATERIAŁÓW WYBUCHOWYCH",
    L"POZIOM DOŚWIADCZENIA",
};

wchar_t* pInvPanelTitleStrings[] = {
    L"Osłona",  // the armor rating of the merc
    L"Ekwip.",  // the weight the merc is carrying
    L"Kamuf.",  // the merc's camouflage rating
};

wchar_t* pShortAttributeStrings[] = {
    L"Zwn",  // the abbreviated version of : agility
    L"Zrc",  // dexterity
    L"Sił",  // strength
    L"Dow",  // leadership
    L"Int",  // wisdom
    L"Doś",  // experience level
    L"Str",  // marksmanship skill
    L"Wyb",  // explosive skill
    L"Mec",  // mechanical skill
    L"Med",  // medical skill
};

wchar_t* pUpperLeftMapScreenStrings[] = {
    L"Przydział",  // the mercs current assignment // *********************NEW****************** as
                   // of June 24, 1998
    L"Kontrakt",   // the contract info about the merc
    L"Zdrowie",    // the health level of the current merc
    L"Morale",     // the morale of the current merc
    L"Stan",       // the condition of the current vehicle
    L"Paliwo",     // the fuel level of the current vehicle
};

wchar_t* pTrainingStrings[] = {
    L"Praktyka",    // tell merc to train self // ****************************NEW*******************
                    // as of June 24, 1998
    L"Samoobrona",  // tell merc to train town // *****************************NEW
                    // ****************** as of June 24, 1998
    L"Instruktor",  // tell merc to act as trainer
    L"Uczeń",  // tell merc to be train by other // **********************NEW******************* as
               // of June 24, 1998
};

wchar_t* pGuardMenuStrings[] = {
    L"Limit ognia:",               // the allowable rate of fire for a merc who is guarding
    L" Agresywny ogień",           // the merc can be aggressive in their choice of fire rates
    L" Oszczędzaj amunicję",       // conserve ammo
    L" Strzelaj w ostateczności",  // fire only when the merc needs to
    L"Inne opcje:",                // other options available to merc
    L" Może się wycofać",          // merc can retreat
    L" Może szukać schronienia",   // merc is allowed to seek cover
    L" Może pomagać partnerom",    // merc can assist teammates
    L"OK",                         // done with this menu
    L"Anuluj",                     // cancel this menu
};

// This string has the same comments as above, however the * denotes the option has been selected by
// the player

wchar_t* pOtherGuardMenuStrings[] = {
    L"Limit ognia:",
    L" *Agresywny ogień*",
    L" *Oszczędzaj amunicję*",
    L" *Strzelaj w ostateczności*",
    L"Inne opcje:",
    L" *Może się wycofać*",
    L" *Może szukać schronienia*",
    L" *Może pomagać partnerom*",
    L"OK",
    L"Anuluj",
};

wchar_t* pAssignMenuStrings[] = {
    L"Służba",     // merc is on active duty
    L"Lekarz",     // the merc is acting as a doctor
    L"Pacjent",    // the merc is receiving medical attention
    L"Pojazd",     // the merc is in a vehicle
    L"Naprawa",    // the merc is repairing items
    L"Szkolenie",  // the merc is training
    L"Anuluj",     // cancel this menu
};

wchar_t* pRemoveMercStrings[] = {
    L"Usuń najemnika",  // remove dead merc from current team
    L"Anuluj",
};

wchar_t* pAttributeMenuStrings[] = {
    L"Siła",        L"Zręczność",     L"Zwinność",       L"Zdrowie",       L"Um. strzeleckie",
    L"Wiedza med.", L"Zn. mechaniki", L"Um. dowodzenia", L"Zn. mat. wyb.", L"Anuluj",
};

wchar_t* pTrainingMenuStrings[] = {
    L"Praktyka",    // train yourself //****************************NEW************************** as
                    // of June 24, 1998
    L"Samoobrona",  // train the town // ****************************NEW *************************
                    // as of June 24, 1998
    L"Instruktor",  // train your teammates // *******************NEW************************** as
                    // of June 24, 1998
    L"Uczeń",   // be trained by an instructor //***************NEW************************** as of
                // June 24, 1998
    L"Anuluj",  // cancel this menu
};

wchar_t* pSquadMenuStrings[] = {
    L"Oddział  1", L"Oddział  2", L"Oddział  3", L"Oddział  4", L"Oddział  5", L"Oddział  6",
    L"Oddział  7", L"Oddział  8", L"Oddział  9", L"Oddział 10", L"Oddział 11", L"Oddział 12",
    L"Oddział 13", L"Oddział 14", L"Oddział 15", L"Oddział 16", L"Oddział 17", L"Oddział 18",
    L"Oddział 19", L"Oddział 20", L"Anuluj",
};

wchar_t* pPersonnelTitle[] = {
    L"Personel",  // the title for the personnel screen/program application
};

wchar_t* pPersonnelScreenStrings[] = {
    L"Zdrowie: ",  // health of merc
    L"Zwinność: ",         L"Zręczność: ",       L"Siła: ",
    L"Um. dowodzenia: ",   L"Inteligencja: ",
    L"Poziom dośw.: ",  // experience level
    L"Um. strzeleckie: ",  L"Zn. mechaniki: ",   L"Zn. mat. wybuchowych: ",
    L"Wiedza medyczna: ",
    L"Zastaw na życie: ",   // amount of medical deposit put down on the merc
    L"Bieżący kontrakt: ",  // cost of current contract
    L"Liczba zabójstw: ",   // number of kills by merc
    L"Liczba asyst: ",      // number of assists on kills by merc
    L"Dzienny koszt:",      // daily cost of merc
    L"Ogólny koszt:",       // total cost of merc
    L"Wartość kontraktu:",  // cost of current contract
    L"Usługi ogółem",       // total service rendered by merc
    L"Zaległa kwota",       // amount left on MERC merc to be paid
    L"Celność:",            // percentage of shots that hit target
    L"Ilość walk:",         // number of battles fought
    L"Ranny(a):",           // number of times merc has been wounded
    L"Umiejętności:",      L"Brak umięjętności",
};

// These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
wchar_t* gzMercSkillText[] = {
    L"Brak umiejętności", L"Otwieranie zamków", L"Walka wręcz", L"Elektronika",
    L"Nocne operacje",    L"Rzucanie",          L"Szkolenie",   L"Ciężka broń",
    L"Broń automatyczna", L"Skradanie się",     L"Oburęczność", L"Kradzieże",
    L"Sztuki walki",      L"Broń biała",        L"Snajper",     L"Kamuflaż",
    L"(Eksp.)",
};

// This is pop up help text for the options that are available to the merc

wchar_t* pTacticalPopupButtonStrings[] = {
    L"W|stań/Idź", L"S|chyl się/Idź", L"Wstań/Biegnij (|R)", L"|Padnij/Czołgaj się", L"Patrz (|L)",
    L"Akcja", L"Rozmawiaj", L"Zbadaj (|C|t|r|l)",

    // Pop up door menu
    L"Otwórz", L"Poszukaj pułapek", L"Użyj wytrychów", L"Wyważ", L"Usuń pułapki",
    L"Zamknij na klucz", L"Otwórz kluczem", L"Użyj ładunku wybuchowego", L"Użyj łomu",
    L"Anuluj (|E|s|c)", L"Zamknij"};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

wchar_t* pDoorTrapStrings[] = {L"nie posiada żadnych pułapek", L"ma założony ładunek wybuchowy",
                               L"jest pod napięciem", L"posiada syrenę alarmową",
                               L"posiada dyskretny alarm"};

// Contract Extension. These are used for the contract extension with AIM mercenaries.

wchar_t* pContractExtendStrings[] = {
    L"dzień",
    L"tydzień",
    L"dwa tygodnie",
};

// On the map screen, there are four columns. This text is popup help text that identifies the
// individual columns.

wchar_t* pMapScreenMouseRegionHelpText[] = {
    L"Wybór postaci",  L"Przydział najemnika", L"Nanieś trasę podróży", L"Kontrakt najemnika",
    L"Usuń najemnika",
    L"Śpij",  // *****************************NEW********************* as of June 29, 1998
};

// volumes of noises

wchar_t* pNoiseVolStr[] = {L"CICHY", L"WYRAŹNY", L"GŁOŚNY", L"BARDZO GŁOŚNY"};

// types of noises

wchar_t* pNoiseTypeStr[] =  // OBSOLETE
    {L"NIEOKREŚLONY DŹWIĘK",
     L"ODGŁOS RUCHU",
     L"ODGŁOS SKRZYPNIĘCIA",
     L"PLUSK",
     L"ODGŁOS UDERZENIA",
     L"STRZAŁ",
     L"WYBUCH",
     L"KRZYK",
     L"ODGŁOS UDERZENIA",
     L"ODGŁOS UDERZENIA",
     L"ŁOMOT",
     L"TRZASK"};

// Directions that are used to report noises

wchar_t* pDirectionStr[] = {L"PŁN-WSCH", L"WSCH", L"PŁD-WSCH", L"PŁD",
                            L"PŁD-ZACH", L"ZACH", L"PŁN-ZACH", L"PŁN"};

// These are the different terrain types.

wchar_t* pLandTypeStrings[] = {
    L"Miasto", L"Droga", L"Otwarty teren", L"Pustynia", L"Las", L"Las", L"Bagno", L"Woda",
    L"Wzgórza", L"Teren nieprzejezdny",
    L"Rzeka",  // river from north to south
    L"Rzeka",  // river from east to west
    L"Terytorium innego kraju",
    // NONE of the following are used for directional travel, just for the sector description.
    L"Tropiki", L"Pola uprawne", L"Otwarty teren, droga", L"Las, droga", L"Las, droga",
    L"Tropiki, droga", L"Las, droga", L"Wybrzeże", L"Góry, droga", L"Wybrzeże, droga",
    L"Pustynia, droga", L"Bagno, droga", L"Las, Rakiety Z-P", L"Pustynia, Rakiety Z-P",
    L"Tropiki, Rakiety Z-P", L"Meduna, Rakiety Z-P",

    // These are descriptions for special sectors
    L"Szpital w Cambrii", L"Lotnisko w Drassen", L"Lotnisko w Medunie", L"Rakiety Z-P",
    L"Kryjówka rebeliantów",  // The rebel base underground in sector A10
    L"Tixa - Lochy",          // The basement of the Tixa Prison (J9)
    L"Gniazdo stworzeń",      // Any mine sector with creatures in it
    L"Orta - Piwnica",        // The basement of Orta (K4)
    L"Tunel",                 // The tunnel access from the maze garden in Meduna
                              // leading to the secret shelter underneath the palace
    L"Schron",                // The shelter underneath the queen's palace
    L"",                      // Unused
};

wchar_t* gpStrategicString[] = {
    L"",                                                           // Unused
    L"%s wykryto w sektorze %c%d, a inny oddział jest w drodze.",  // STR_DETECTED_SINGULAR
    L"%s wykryto w sektorze %c%d, a inne oddziały są w drodze.",   // STR_DETECTED_PLURAL
    L"Chcesz skoordynować jednoczesne przybycie?",                 // STR_COORDINATE

    // Dialog strings for enemies.

    L"Wróg daje ci szansę się poddać.",                         // STR_ENEMY_SURRENDER_OFFER
    L"Wróg schwytał resztę twoich nieprzytomnych najemników.",  // STR_ENEMY_CAPTURED

    // The text that goes on the autoresolve buttons

    L"Odwrót",  // The retreat button				//STR_AR_RETREAT_BUTTON
    L"OK",      // The done button				//STR_AR_DONE_BUTTON

    // The headers are for the autoresolve type (MUST BE UPPERCASE)

    L"OBRONA",   // STR_AR_DEFEND_HEADER
    L"ATAK",     // STR_AR_ATTACK_HEADER
    L"STARCIE",  // STR_AR_ENCOUNTER_HEADER
    L"Sektor",   // The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

    // The battle ending conditions

    L"ZWYCIĘSTWO!",   // STR_AR_OVER_VICTORY
    L"PORAŻKA!",      // STR_AR_OVER_DEFEAT
    L"KAPITULACJA!",  // STR_AR_OVER_SURRENDERED
    L"NIEWOLA!",      // STR_AR_OVER_CAPTURED
    L"ODWRÓT!",       // STR_AR_OVER_RETREATED

    // These are the labels for the different types of enemies we fight in autoresolve.

    L"Samoobrona",     // STR_AR_MILITIA_NAME,
    L"Elity",          // STR_AR_ELITE_NAME,
    L"Żołnierze",      // STR_AR_TROOP_NAME,
    L"Administrator",  // STR_AR_ADMINISTRATOR_NAME,
    L"Stworzenie",     // STR_AR_CREATURE_NAME,

    // Label for the length of time the battle took

    L"Czas trwania",  // STR_AR_TIME_ELAPSED,

    // Labels for status of merc if retreating.  (UPPERCASE)

    L"WYCOFAŁ(A) SIĘ",  // STR_AR_MERC_RETREATED,
    L"WYCOFUJE SIĘ",    // STR_AR_MERC_RETREATING,
    L"WYCOFAJ SIĘ",     // STR_AR_MERC_RETREAT,

    // PRE BATTLE INTERFACE STRINGS
    // Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
    // a system that automatically resolves the combat for the player without having to do anything.
    // These strings must be short (two lines -- 6-8 chars per line)

    L"Rozst. autom.",   // STR_PB_AUTORESOLVE_BTN,
    L"Idź do sektora",  // STR_PB_GOTOSECTOR_BTN,
    L"Wycof. ludzi",    // STR_PB_RETREATMERCS_BTN,

    // The different headers(titles) for the prebattle interface.
    L"STARCIE Z WROGIEM",  // STR_PB_ENEMYENCOUNTER_HEADER,
    L"INWAZJA WROGA",      // STR_PB_ENEMYINVASION_HEADER, // 30
    L"ZASADZKA WROGA",
    L"WEJŚCIE DO WROGIEGO SEKTORA",
    L"ATAK STWORÓW",
    L"ATAK DZIKICH KOTÓW",                  // STR_PB_BLOODCATAMBUSH_HEADER
    L"WEJŚCIE DO LEGOWISKA DZIKICH KOTÓW",  // STR_PB_ENTERINGBLOODCATLAIR_HEADER

    // Various single words for direct translation.  The Civilians represent the civilian
    // militia occupying the sector being attacked.  Limited to 9-10 chars

    L"Położenie",
    L"Wrogowie",
    L"Najemnicy",
    L"Samoobrona",
    L"Stwory",
    L"Dzikie koty",
    L"Sektor",
    L"Brak",  // If there are no uninvolved mercs in this fight.
    L"N/D",   // Acronym of Not Applicable N/A
    L"d",     // One letter abbreviation of day
    L"g",     // One letter abbreviation of hour

    // TACTICAL PLACEMENT USER INTERFACE STRINGS
    // The four buttons

    L"Wyczyść",
    L"Rozprosz",
    L"Zgrupuj",
    L"OK",

    // The help text for the four buttons.  Use \n to denote new line (just like enter).

    L"Kasuje wszystkie pozy|cje najemników, \ni pozwala ponownie je wprowadzić.",
    L"Po każdym naciśnięciu rozmie|szcza\nlosowo twoich najemników.",
    L"|Grupuje najemników w wybranym miejscu.",
    L"Kliknij ten klawisz gdy już rozmieścisz \nswoich najemników. (|E|n|t|e|r)",
    L"Musisz rozmieścić wszystkich najemników \nzanim rozpoczniesz walkę.",

    // Various strings (translate word for word)

    L"Sektor",
    L"Wybierz początkowe pozycje",

    // Strings used for various popup message boxes.  Can be as long as desired.

    L"To miejsce nie jest zbyt dobre. Jest niedostępne. Spróbuj gdzie indziej.",
    L"Rozmieść swoich najemników na podświetlonej części mapy.",

    // This message is for mercs arriving in sectors.  Ex:  Red has arrived in sector A9.
    // Don't uppercase first character, or add spaces on either end.

    L"przybył(a) do sektora",

    // These entries are for button popup help text for the prebattle interface.  All popup help
    // text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
    L"|Automatycznie prowadzi walkę za ciebie,\nnie ładując planszy.",
    L"Atakując sektor wroga nie można automatycznie rozstrzygnąć walki.",
    L"Wejście do s|ektora by nawiązać walkę z wrogiem.",
    L"Wycofuje oddział do sąsiedniego sekto|ra.",             // singular version
    L"Wycofuje wszystkie oddziały do sąsiedniego sekto|ra.",  // multiple groups with same previous
                                                              // sector

    // various popup messages for battle conditions.

    //%c%d is the sector -- ex:  A9
    L"Nieprzyjaciel zatakował oddziały samoobrony w sektorze %c%d.",
    //%c%d is the sector -- ex:  A9
    L"Stworzenia zaatakowały oddziały samoobrony w sektorze %c%d.",
    // 1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
    // Note:  the minimum number of civilians eaten will be two.
    L"Stworzenia zatakowały i zabiły %d cywili w sektorze %s.",
    //%c%d is the sector -- ex:  A9
    L"Nieprzyjaciel zatakował twoich najemników w sektorze %s.  Żaden z twoich najemników nie może "
    L"walczyć!",
    //%c%d is the sector -- ex:  A9
    L"Stworzenia zatakowały twoich najemników w sektorze %s.  Żaden z twoich najemników nie może "
    L"walczyć!",

};

wchar_t* gpGameClockString[] = {
    // This is the day represented in the game clock.  Must be very short, 4 characters max.
    L"Dzień",
};

// When the merc finds a key, they can get a description of it which
// tells them where and when they found it.
wchar_t* sKeyDescriptionStrings[2] = {
    L"Zn. w sektorze:",
    L"Zn. w dniu:",
};

// The headers used to describe various weapon statistics.

int16_t gWeaponStatsDesc[][14] = {
    L"Waga (%s):",  // change kg to another weight unit if your standard is not kilograms, and TELL
                    // SIR-TECH!
    L"Stan:",
    L"Ilość:",  // Number of bullets left in a magazine
    L"Zas.:",   // Range
    L"Siła:",   // Damage
    L"PA:",     // abbreviation for Action Points
    L"",
    L"=",
    L"=",
};

// The headers used for the merc's money.

int16_t gMoneyStatsDesc[][13] = {
    L"Kwota",
    L"Pozostało:",  // this is the overall balance
    L"Kwota",
    L"Wydzielić:",  // the amount he wants to separate from the overall balance to get two piles of
                    // money

    L"Bieżące",
    L"Saldo",
    L"Kwota do",
    L"podjęcia",
};

// The health of various creatures, enemies, characters in the game. The numbers following each are
// for comment only, but represent the precentage of points remaining.

uint16_t zHealthStr[][13] = {
    L"UMIERAJĄCY",  //	>= 0
    L"KRYTYCZNY",   //	>= 15
    L"KIEPSKI",     //	>= 30
    L"RANNY",       //	>= 45
    L"ZDROWY",      //	>= 60
    L"SILNY",       // 	>= 75
    L"DOSKONAŁY",   // 	>= 90
};

wchar_t* gzMoneyAmounts[6] = {
    L"$1000", L"$100", L"$10", L"OK", L"Wydziel", L"Podejmij",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
int16_t gzProsLabel[10] = {
    L"Zalety:",
};

int16_t gzConsLabel[10] = {
    L"Wady:",
};

// Conversation options a player has when encountering an NPC
uint16_t zTalkMenuStrings[6][SMALL_STRING_LENGTH] = {
    L"Powtórz",       // meaning "Repeat yourself"
    L"Przyjaźnie",    // approach in a friendly
    L"Bezpośrednio",  // approach directly - let's get down to business
    L"Groźnie",       // approach threateningly - talk now, or I'll blow your face off
    L"Daj",          L"Rekrutuj",
};

// Some NPCs buy, sell or repair items. These different options are available for those NPCs as
// well.
uint16_t zDealerStrings[4][SMALL_STRING_LENGTH] = {
    L"Kup/Sprzedaj",
    L"Kup",
    L"Sprzedaj",
    L"Napraw",
};

uint16_t zDialogActions[1][SMALL_STRING_LENGTH] = {
    L"OK",
};

// These are vehicles in the game.

wchar_t* pVehicleStrings[] = {
    L"Eldorado",
    L"Hummer",  // a hummer jeep/truck -- military vehicle
    L"Furgonetka z lodami",
    L"Jeep",
    L"Czołg",
    L"Helikopter",
};

wchar_t* pShortVehicleStrings[] = {
    L"Eldor.",
    L"Hummer",  // the HMVV
    L"Furg.",  L"Jeep", L"Czołg",
    L"Heli.",  // the helicopter
};

wchar_t* zVehicleName[] = {
    L"Eldorado",
    L"Hummer",  // a military jeep. This is a brand name.
    L"Furg.",   // Ice cream truck
    L"Jeep",     L"Czołg",
    L"Heli.",  // an abbreviation for Helicopter
};

// These are messages Used in the Tactical Screen

uint16_t TacticalStr[][MED_STRING_LENGTH] = {
    L"Nalot", L"Udzielić automatycznie pierwszej pomocy?",

    // CAMFIELD NUKE THIS and add quote #66.

    L"%s zauważył(a) że dostawa jest niekompletna.",

    // The %s is a string from pDoorTrapStrings

    L"Zamek %s.", L"Brak zamka.", L"Sukces!", L"Niepowodzenie.", L"Sukces!", L"Niepowodzenie.",
    L"Zamek nie ma pułapek.", L"Sukces!",
    // The %s is a merc name
    L"%s nie posiada odpowiedniego klucza.", L"Zamek został rozbrojony.", L"Zamek nie ma pułapek.",
    L"Zamknięte.", L"DRZWI", L"ZABEZP.", L"ZAMKNIĘTE", L"OTWARTE", L"ROZWALONE",
    L"Tu jest przełącznik. Włączyć go?", L"Rozbroić pułapkę?", L"Poprz...", L"Nast...",
    L"Więcej...",

    // In the next 2 strings, %s is an item name

    L"%s - położono na ziemi.", L"%s - przekazano do - %s.",

    // In the next 2 strings, %s is a name

    L"%s otrzymał(a) całą zapłatę.", L"%s - należność wobec niej/niego wynosi jeszcze %d.",
    L"Wybierz częstotliwość sygnału detonującego:",  // in this case, frequency refers to a radio
                                                     // signal
    L"Ile tur do eksplozji:",                     // how much time, in turns, until the bomb blows
    L"Ustaw częstotliwość zdalnego detonatora:",  // in this case, frequency refers to a radio
                                                  // signal
    L"Rozbroić pułapkę?", L"Usunąć niebieską flagę?", L"Umieścić tutaj niebieską flagę?",
    L"Kończąca tura",

    // In the next string, %s is a name. Stance refers to way they are standing.

    L"Na pewno chcesz zaatakować - %s?", L"Pojazdy nie mogą zmieniać pozycji.",
    L"Robot nie może zmieniać pozycji.",

    // In the next 3 strings, %s is a name

    L"%s nie może zmienić pozycji w tym miejscu.", L"%s nie może tu otrzymać pierwszej pomocy.",
    L"%s nie potrzebuje pierwszej pomocy.", L"Nie można ruszyć w to miejsce.",
    L"Oddział jest już kompletny. Nie ma miejsca dla nowych rekrutów.",  // there's no room for a
                                                                         // recruit on the player's
                                                                         // team

    // In the next string, %s is a name

    L"%s pracuje już dla ciebie.",

    // Here %s is a name and %d is a number

    L"%s - należność wobec niej/niego wynosi %d$.",

    // In the next string, %s is a name

    L"%s - Eskortować tą osobę?",

    // In the next string, the first %s is a name and the second %s is an amount of money (including
    // $ sign)

    L"%s - Zatrudnić tą osobę za %s dziennie?",

    // This line is used repeatedly to ask player if they wish to participate in a boxing match.

    L"Chcesz walczyć?",

    // In the next string, the first %s is an item name and the
    // second %s is an amount of money (including $ sign)

    L"%s - Kupić to za %s?",

    // In the next string, %s is a name

    L"%s jest pod eskortą oddziału %d.",

    // These messages are displayed during play to alert the player to a particular situation

    L"ZACIĘTA",                                // weapon is jammed.
    L"Robot potrzebuje amunicji kaliber %s.",  // Robot is out of ammo
    L"Rzucić tam? To niemożliwe.",             // Merc can't throw to the destination he selected

    // These are different buttons that the player can turn on and off.

    L"Skradanie się (|Z)", L"Ekran |Mapy", L"Koniec tury (|D)", L"Rozmowa", L"Wycisz",
    L"Pozycja do góry (|P|g|U|p)", L"Poziom kursora (|T|a|b)", L"Wspinaj się / Zeskocz",
    L"Pozycja w dół (|P|g|D|n)", L"Badać (|C|t|r|l)", L"Poprzedni najemnik",
    L"Następny najemnik (|S|p|a|c|j|a)", L"|Opcje", L"Ciągły ogień (|B)", L"Spójrz/Obróć się (|L)",
    L"Zdrowie: %d/%d\nEnergia: %d/%d\nMorale: %s",
    L"Co?",   // this means "what?"
    L"Kont",  // an abbrieviation for "Continued"
    L"%s ma włączone potwierdzenia głosowe.", L"%s ma wyłączone potwierdzenia głosowe.",
    L"Stan: %d/%d\nPaliwo: %d/%d", L"Wysiądź z pojazdu",
    L"Zmień oddział ( |S|h|i|f|t |S|p|a|c|j|a )", L"Prowadź",
    L"N/D",  // this is an acronym for "Not Applicable."
    L"Użyj ( Walka wręcz )", L"Użyj ( Broni palnej )", L"Użyj ( Broni białej )",
    L"Użyj ( Mat. wybuchowych )", L"Użyj ( Apteczki )", L"(Łap)", L"(Przeładuj)", L"(Daj)",
    L"%s - pułapka została uruchomiona.", L"%s przybył(a) na miejsce.",
    L"%s stracił(a) wszystkie Punkty Akcji.", L"%s jest nieosiągalny(na).",
    L"%s ma już założone opatrunki.", L"%s nie ma bandaży.", L"Wróg w sektorze!",
    L"Nie ma wroga w zasięgu wzroku.", L"Zbyt mało Punktów Akcji.",
    L"Nikt nie używa zdalnego sterowania.", L"Ciągły ogień opróżnił magazynek!", L"ŻOŁNIERZ",
    L"STWÓR", L"SAMOOBRONA", L"CYWIL", L"Wyjście z sektora", L"OK", L"Anuluj", L"Wybrany najemnik",
    L"Wszyscy najemnicy w oddziale", L"Idź do sektora", L"Otwórz mapę",
    L"Nie można opuścić sektora z tej strony.", L"%s jest zbyt daleko.", L"Usuń korony drzew",
    L"Pokaż korony drzew",
    L"WRONA",  // Crow, as in the large black bird
    L"SZYJA", L"GŁOWA", L"TUŁÓW", L"NOGI", L"Powiedzieć królowej to, co chce wiedzieć?",
    L"Wzór odcisku palca pobrany", L"Niewłaściwy wzór odcisku palca. Broń bezużyteczna.",
    L"Cel osiągnięty", L"Droga zablokowana",
    L"Wpłata/Podjęcie pieniędzy",  // Help text over the $ button on the Single Merc Panel
    L"Nikt nie potrzebuje pierwszej pomocy.",
    L"Zac.",                       // Short form of JAMMED, for small inv slots
    L"Nie można się tam dostać.",  // used ( now ) for when we click on a cliff
    L"Przejście zablokowane. Czy chcesz zamienić się miejscami z tą osobą?",
    L"Osoba nie chce się przesunąć.",
    // In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
    L"Zgadzasz się zapłacić %s?", L"Zgadzasz się na darmowe leczenie?",
    L"Zgadasz się na małżeństwo z Darylem?", L"Kółko na klucze",
    L"Nie możesz tego zrobić z eskortowaną osobą.", L"Oszczędzić Krotta?", L"Poza zasięgiem broni",
    L"Górnik", L"Pojazdem można podróżować tylko pomiędzy sektorami",
    L"Teraz nie można automatycznie udzielić pierwszej pomocy", L"Przejście zablokowane dla - %s",
    L"Twoi najemnicy, schwytani przez żołnierzy Deidranny, są tutaj uwięzieni!",
    L"Zamek został trafiony", L"Zamek został zniszczony",
    L"Ktoś inny majstruje przy tych drzwiach.", L"Stan: %d/%d\nPaliwo: %d/%d",
    L"%s nie widzi - %s.",  // Cannot see person trying to talk to
};

// Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different
// circumstances in the "exiting sector" interface.
wchar_t* pExitingSectorHelpText[] = {
    // Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box
    // is checked.
    L"Jeśli zaznaczysz tę opcję, to sąsiedni sektor zostanie natychmiast załadowany.",
    L"Jeśli zaznaczysz tę opcję, to na czas podróży pojawi się automatycznie ekran mapy.",

    // If you attempt to leave a sector when you have multiple squads in a hostile sector.
    L"Ten sektor jest okupowany przez wroga i nie możesz tu zostawić najemników.\nMusisz uporać "
    L"się z tą sytuacją zanim załadujesz inny sektor.",

    // Because you only have one squad in the sector, and the "move all" option is checked, the "go
    // to sector" option is locked to on. The helptext explains why it is locked.
    L"Gdy wyprowadzisz swoich pozostałych najemników z tego sektora,\nsąsiedni sektor zostanie "
    L"automatycznie załadowany.",
    L"Gdy wyprowadzisz swoich pozostałych najemników z tego sektora,\nto na czas podróży pojawi "
    L"się automatycznie ekran mapy.",

    // If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being
    // escorted.  The "single" button is disabled.
    L"%s jest pod eskortą twoich najemników i nie może bez nich opuścić tego sektora.",

    // If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc
    // will be prohibited from leaving alone. There are several strings depending on the gender of
    // the merc and how many EPCs are in the squad. DO NOT USE THE NEWLINE HERE AS IT IS USED FOR
    // BOTH HELPTEXT AND SCREEN MESSAGES!
    L"%s nie może sam opuścić tego sektora, gdyż %s jest pod jego eskortą.",  // male singular
    L"%s nie może sama opuścić tego sektora, gdyż %s jest pod jej eskortą.",  // female singular
    L"%s nie może sam opuścić tego sektora, gdyż eskortuje inne osoby.",      // male plural
    L"%s nie może sama opuścić tego sektora, gdyż eskortuje inne osoby.",     // female plural

    // If one or more of your mercs in the selected squad aren't in range of the traversal area,
    // then the  "move all" option is disabled, and this helptext explains why.
    L"Wszyscy twoi najemnicy muszą być w pobliżu,\naby oddział mógł się przemieszczać.",

    L"",  // UNUSED

    // Standard helptext for single movement.  Explains what will happen (splitting the squad)
    L"Jeśli zaznaczysz tę opcję, %s będzie podróżować w pojedynkę\ni automatycznie znajdzie się w "
    L"osobnym oddziale.",

    // Standard helptext for all movement.  Explains what will happen (moving the squad)
    L"Jeśli zaznaczysz tę opcję, aktualnie\nwybrany oddział opuści ten sektor.",

    // This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC
    // selected and you attempt to tactically traverse the EPC while the escorting mercs aren't near
    // enough (or dead, dying, or unconscious), this message will appear and the "exiting sector"
    // interface will not appear.  This is just like the situation where This string is special, as
    // it is not used as helptext.  Do not use the special newline character (\n) for this string.
    L"%s jest pod eskortą twoich najemników i nie może bez nich opuścić tego sektora. Aby opuścić "
    L"sektor twoi najemnicy muszą być w pobliżu.",
};

wchar_t* pRepairStrings[] = {
    L"Wyposażenie",      // tell merc to repair items in inventory
    L"Baza rakiet Z-P",  // tell merc to repair SAM site - SAM is an acronym for Surface to Air
                         // Missile
    L"Anuluj",           // cancel this menu
    L"Robot",            // repair the robot
};

// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

wchar_t* sPreStatBuildString[] = {
    L"traci",    // the merc has lost a statistic
    L"zyskuje",  // the merc has gained a statistic
    L"pkt.",     // singular
    L"pkt.",     // plural
    L"pkt.",     // singular
    L"pkt.",     // plural
};

wchar_t* sStatGainStrings[] = {
    L"zdrowia.",
    L"zwinności.",
    L"zręczności.",
    L"inteligencji.",
    L"umiejętności medycznych.",
    L"umiejętności w dziedzinie materiałów wybuchowych.",
    L"umiejętności w dziedzinie mechaniki.",
    L"umiejętności strzeleckich.",
    L"doświadczenia.",
    L"siły.",
    L"umiejętności dowodzenia.",
};

wchar_t* pHelicopterEtaStrings[] = {
    L"Całkowita trasa:  ",  // total distance for helicopter to travel
    L" Bezp.:   ",          // distance to travel to destination
    L" Niebezp.:",          // distance to return from destination to airport
    L"Całkowity koszt: ",   // total cost of trip by helicopter
    L"PCP:  ",              // ETA is an acronym for "estimated time of arrival"
    L"Helikopter ma mało paliwa i musi wylądować na terenie wroga.",  // warning that the sector the
                                                                      // helicopter is going to use
                                                                      // for refueling is under
                                                                      // enemy control ->
    L"Pasażerowie: ",
    L"Wybór Skyridera lub pasażerów?",
    L"Skyrider",
    L"Pasażerowie",
};

wchar_t* sMapLevelString[] = {
    L"Poziom:",  // what level below the ground is the player viewing in mapscreen
};

wchar_t* gsLoyalString[] = {
    L"Lojalności",  // the loyalty rating of a town ie : Loyal 53%
};

// error message for when player is trying to give a merc a travel order while he's underground.

wchar_t* gsUndergroundString[] = {
    L"nie można wydawać rozkazów podróży pod ziemią.",
};

wchar_t* gsTimeStrings[] = {
    L"g",  // hours abbreviation
    L"m",  // minutes abbreviation
    L"s",  // seconds abbreviation
    L"d",  // days abbreviation
};

// text for the various facilities in the sector

wchar_t* sFacilitiesStrings[] = {
    L"Brak",       L"Szpital", L"Przemysł", L"Więzienie", L"Baza wojskowa", L"Lotnisko",
    L"Strzelnica",  // a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

wchar_t* pMapPopUpInventoryText[] = {
    L"Inwentarz",
    L"Zamknij",
};

// town strings

wchar_t* pwTownInfoStrings[] = {
    L"Rozmiar",               // 0 // size of the town in sectors
    L"",                      // blank line, required
    L"Pod kontrolą",          // how much of town is controlled
    L"Brak",                  // none of this town
    L"Przynależna kopalnia",  // mine associated with this town
    L"Lojalność",             // 5 // the loyalty level of this town
    L"Wyszkolonych",          // the forces in the town trained by the player
    L"",
    L"Główne obiekty",    // main facilities in this town
    L"Poziom",            // the training level of civilians in this town
    L"Szkolenie cywili",  // 10 // state of civilian training in town
    L"Samoobrona",        // the state of the trained civilians in the town
};

// Mine strings

wchar_t* pwMineStrings[] = {
    L"Kopalnia",  // 0
    L"Srebro",
    L"Złoto",
    L"Dzienna produkcja",
    L"Możliwa produkcja",
    L"Opuszczona",  // 5
    L"Zamknięta",
    L"Na wyczerpaniu",
    L"Produkuje",
    L"Stan",
    L"Tempo produkcji",
    L"Typ złoża",  // 10
    L"Kontrola miasta",
    L"Lojalność miasta",
    //	L"Górnicy",
};

// blank sector strings

wchar_t* pwMiscSectorStrings[] = {
    L"Siły wroga", L"Sektor", L"Przedmiotów", L"Nieznane", L"Pod kontrolą", L"Tak", L"Nie",
};

// error strings for inventory

wchar_t* pMapInventoryErrorString[] = {
    L"%s jest zbyt daleko.",              // Merc is in sector with item but not close enough
    L"Nie można wybrać tego najemnika.",  // MARK CARTER
    L"%s nie może stąd zabrać tego przedmiotu, gdyż nie jest w tym sektorze.",
    L"Podczas walki nie można korzystać z tego panelu.",
    L"Podczas walki nie można korzystać z tego panelu.",
    L"%s nie może tu zostawić tego przedmiotu, gdyż nie jest w tym sektorze.",
};

wchar_t* pMapInventoryStrings[] = {
    L"Położenie",          // sector these items are in
    L"Razem przedmiotów",  // total number of items in sector
};

// help text for the user

wchar_t* pMapScreenFastHelpTextList[] = {
    L"Kliknij w kolumnie 'Przydz.', aby przydzielić najemnika do innego oddziału lub wybranego "
    L"zadania.",
    L"Aby wyznaczyć najemnikowi cel w innym sektorze, kliknij pole w kolumnie 'Cel'.",
    L"Gdy najemnicy otrzymają już rozkaz przemieszczenia się, kompresja czasu pozwala im szybciej "
    L"dotrzeć na miejsce.",
    L"Kliknij lewym klawiszem aby wybrać sektor. Kliknij ponownie aby wydać najemnikom rozkazy "
    L"przemieszczenia, lub kliknij prawym klawiszem by uzyskać informacje o sektorze.",
    L"Naciśnij w dowolnym momencie klawisz 'H' by wyświetlić okienko pomocy.",
    L"Próbny tekst",
    L"Próbny tekst",
    L"Próbny tekst",
    L"Próbny tekst",
    L"Niewiele możesz tu zrobić, dopóki najemnicy nie przylecą do Arulco. Gdy już zbierzesz swój "
    L"oddział, kliknij przycisk Kompresji Czasu, w prawym dolnym rogu. W ten sposób twoi najemnicy "
    L"szybciej dotrą na miejsce.",
};

// movement menu text

wchar_t* pMovementMenuStrings[] = {
    L"Przemieść najemników",  // title for movement box
    L"Nanieś trasę podróży",  // done with movement menu, start plotting movement
    L"Anuluj",                // cancel this menu
    L"Inni",                  // title for group of mercs not on squads nor in vehicles
};

wchar_t* pUpdateMercStrings[] = {
    L"Oj:",                           // an error has occured
    L"Wygasł kontrakt najemników:",   // this pop up came up due to a merc contract ending
    L"Najemnicy wypełnili zadanie:",  // this pop up....due to more than one merc finishing
                                      // assignments
    L"Najemnicy wrócili do pracy:",   // this pop up ....due to more than one merc waking up and
                                      // returing to work
    L"Odpoczywający najemnicy:",  // this pop up ....due to more than one merc being tired and going
                                  // to sleep
    L"Wkrótce wygasną kontrakty:",  // this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

wchar_t* pMapScreenBorderButtonHelpText[] = {
    L"Pokaż miasta (|W)",
    L"Pokaż kopalnie (|M)",
    L"Pokaż oddziały i wrogów (|T)",
    L"Pokaż przestrzeń powietrzną (|A)",
    L"Pokaż przedmioty (|I)",
    L"Pokaż samoobronę i wrogów (|Z)",
};

wchar_t* pMapScreenBottomFastHelp[] = {
    L"|Laptop", L"Ekran taktyczny (|E|s|c)", L"|Opcje",
    L"Kompresja czasu (|+)",  // time compress more
    L"Kompresja czasu (|-)",  // time compress less
    L"Poprzedni komunikat (|S|t|r|z|a|ł|k|a |w |g|ó|r|ę)\nPoprzednia strona (|P|g|U|p)",  // previous
                                                                                          // message
                                                                                          // in
                                                                                          // scrollable
                                                                                          // list
    L"Następny komunikat (|S|t|r|z|a|ł|k|a |w |d|ó|ł)\nNastępna strona (|P|g|D|n)",  // next message
                                                                                     // in the
                                                                                     // scrollable
                                                                                     // list
    L"Włącz/Wyłącz kompresję czasu (|S|p|a|c|j|a)",  // start/stop time compression
};

wchar_t* pMapScreenBottomText[] = {
    L"Saldo dostępne",  // current balance in player bank account
};

wchar_t* pMercDeadString[] = {
    L"%s nie żyje.",
};

wchar_t* pDayStrings[] = {
    L"Dzień",
};

// the list of email sender names

wchar_t* pSenderNameList[] = {
    L"Enrico",
    L"Psych Pro Inc",
    L"Pomoc",
    L"Psych Pro Inc",
    L"Speck",
    L"R.I.S.",
    L"Barry",
    L"Blood",
    L"Lynx",
    L"Grizzly",
    L"Vicki",
    L"Trevor",
    L"Grunty",
    L"Ivan",
    L"Steroid",
    L"Igor",
    L"Shadow",
    L"Red",
    L"Reaper",
    L"Fidel",
    L"Fox",
    L"Sidney",
    L"Gus",
    L"Buns",
    L"Ice",
    L"Spider",
    L"Cliff",
    L"Bull",
    L"Hitman",
    L"Buzz",
    L"Raider",
    L"Raven",
    L"Static",
    L"Len",
    L"Danny",
    L"Magic",
    L"Stephan",
    L"Scully",
    L"Malice",
    L"Dr.Q",
    L"Nails",
    L"Thor",
    L"Scope",
    L"Wolf",
    L"MD",
    L"Meltdown",
    //----------
    L"M.I.S. Ubezpieczenia",
    L"Bobby Ray",
    L"Kingpin",
    L"John Kulba",
    L"A.I.M.",
};

// next/prev strings

wchar_t* pTraverseStrings[] = {
    L"Poprzedni",
    L"Następny",
};

// new mail notify string

wchar_t* pNewMailStrings[] = {
    L"Masz nową pocztę...",
};

// confirm player's intent to delete messages

wchar_t* pDeleteMailStrings[] = {
    L"Usunąć wiadomość?",
    L"Usunąć wiadomość?",
};

// the sort header strings

wchar_t* pEmailHeaders[] = {
    L"Od:",
    L"Temat:",
    L"Dzień:",
};

// email titlebar text

wchar_t* pEmailTitleText[] = {
    L"Skrzynka odbiorcza",
};

// the financial screen strings
wchar_t* pFinanceTitle[] = {
    L"Księgowy Plus",  // the name we made up for the financial program in the game
};

wchar_t* pFinanceSummary[] = {
    L"Wypłata:",  // credit (subtract from) to player's account
    L"Wpłata:",   // debit (add to) to player's account
    L"Wczorajsze wpływy:",
    L"Wczorajsze dodatkowe wpływy:",
    L"Wczorajsze wydatki:",
    L"Saldo na koniec dnia:",
    L"Dzisiejsze wpływy:",
    L"Dzisiejsze dodatkowe wpływy:",
    L"Dzisiejsze wydatki:",
    L"Saldo dostępne:",
    L"Przewidywane wpływy:",
    L"Przewidywane saldo:",  // projected balance for player for tommorow
};

// headers to each list in financial screen

wchar_t* pFinanceHeaders[] = {
    L"Dzień",        // the day column
    L"Ma",           // the credits column
    L"Winien",       // the debits column
    L"Transakcja",   // transaction type - see TransactionText below
    L"Saldo",        // balance at this point in time
    L"Strona",       // page number
    L"Dzień (dni)",  // the day(s) of transactions this page displays
};

wchar_t* pTransactionText[] = {
    L"Narosłe odsetki",  // interest the player has accumulated so far
    L"Anonimowa wpłata",
    L"Koszt transakcji",
    L"Wynajęto -",                // Merc was hired
    L"Zakupy u Bobby'ego Ray'a",  // Bobby Ray is the name of an arms dealer
    L"Uregulowanie rachunków w M.E.R.C.",
    L"Zastaw na życie dla - %s",  // medical deposit for merc
    L"Analiza profilu w IMP",     // IMP is the acronym for International Mercenary Profiling
    L"Ubezpieczneie dla - %s",
    L"Redukcja ubezp. dla - %s",
    L"Przedł. ubezp. dla - %s",  // johnny contract extended
    L"Anulowano ubezp. dla - %s",
    L"Odszkodowanie za - %s",  // insurance claim for merc
    L"1 dzień",                // merc's contract extended for a day
    L"1 tydzień",              // merc's contract extended for a week
    L"2 tygodnie",             // ... for 2 weeks
    L"Przychód z kopalni",
    L"",  // String nuked
    L"Zakup kwiatów",
    L"Pełny zwrot zastawu za - %s",
    L"Częściowy zwrot zastawu za - %s",
    L"Brak zwrotu zastawu za - %s",
    L"Zapłata dla - %s",           // %s is the name of the npc being paid
    L"Transfer funduszy do - %s",  // transfer funds to a merc
    L"Transfer funduszy od - %s",  // transfer funds from a merc
    L"Samoobrona w - %s",          // initial cost to equip a town's militia
    L"Zakupy u - %s.",  // is used for the Shop keeper interface.  The dealers name will be appended
                        // to the end of the string.
    L"%s wpłacił(a) pieniądze.",
};

wchar_t* pTransactionAlternateText[] = {
    L"Ubezpieczenie dla -",                 // insurance for a merc
    L"Przedł. kontrakt z - %s o 1 dzień.",  // entend mercs contract by a day
    L"Przedł. kontrakt z - %s o 1 tydzień.",
    L"Przedł. kontrakt z - %s o 2 tygodnie.",
};

// helicopter pilot payment

wchar_t* pSkyriderText[] = {
    L"Skyriderowi zapłacono %d$",                // skyrider was paid an amount of money
    L"Skyriderowi trzeba jeszcze zapłacić %d$",  // skyrider is still owed an amount of money
    L"Skyrider zatankował",                      // skyrider has finished refueling
    L"",                                         // unused
    L"",                                         // unused
    L"Skyrider jest gotów do kolejnego lotu.",   // Skyrider was grounded but has been freed
    L"Skyrider nie ma pasażerów. Jeśli chcesz przetransportować najemników, zmień ich przydział na "
    L"POJAZD/HELIKOPTER.",
};

// strings for different levels of merc morale

wchar_t* pMoralStrings[] = {
    L"Świetne", L"Dobre", L"Stabilne", L"Słabe", L"Panika", L"Złe",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.

wchar_t* pLeftEquipmentString[] = {
    L"%s - jego/jej sprzęt jest już w Omercie( A9 ).",
    L"%s - jego/jej sprzęt jest już w Drassen( B13 ).",
};

// Status that appears on the Map Screen

wchar_t* pMapScreenStatusStrings[] = {
    L"Zdrowie", L"Energia", L"Morale",
    L"Stan",    // the condition of the current vehicle (its "health")
    L"Paliwo",  // the fuel level of the current vehicle (its "energy")
};

wchar_t* pMapScreenPrevNextCharButtonHelpText[] = {
    L"Poprzedni najemnik (|S|t|r|z|a|ł|k|a |w |l|e|w|o)",   // previous merc in the list
    L"Następny najemnik (|S|t|r|z|a|ł|k|a |w |p|r|a|w|o)",  // next merc in the list
};

wchar_t* pEtaString[] = {
    L"PCP:",  // eta is an acronym for Estimated Time of Arrival
};

wchar_t* pTrashItemText[] = {
    L"Więcej tego nie zobaczysz. Czy na pewno chcesz to zrobić?",  // do you want to continue and
                                                                   // lose the item forever
    L"To wygląda na coś NAPRAWDĘ ważnego. Czy NA PEWNO chcesz to zniszczyć?",  // does the user
                                                                               // REALLY want to
                                                                               // trash this item
};

wchar_t* pMapErrorString[] = {
    L"Oddział nie może się przemieszczać, jeśli któryś z najemników śpi.",

    // 1-5
    L"Najpierw wyprowadź oddział na powierzchnię.",
    L"Rozkazy przemieszczenia? To jest sektor wroga!",
    L"Aby podróżować najemnicy muszą być przydzieleni do oddziału lub pojazdu.",
    L"Nie masz jeszcze ludzi.",                   // you have no members, can't do anything
    L"Najemnik nie może wypełnić tego rozkazu.",  // merc can't comply with your order
                                                  // 6-10
    L"musi mieć eskortę, aby się przemieszczać. Umieść go w oddziale z eskortą.",  // merc can't
                                                                                   // move
                                                                                   // unescorted ..
                                                                                   // for a male
    L"musi mieć eskortę, aby się przemieszczać. Umieść ją w oddziale z eskortą.",  // for a female
    L"Najemnik nie przybył jeszcze do Arulco!",
    L"Wygląda na to, że trzeba wpierw uregulować sprawy kontraktu.",
    L"",
    // 11-15
    L"Rozkazy przemieszczenia? Trwa walka!",
    L"Zaatakowały cię dzikie koty, w sektorze %s!",
    L"W sektorze I16 znajduje się coś, co wygląda na legowisko dzikich kotów!",
    L"",
    L"Baza rakiet Ziemia-Powietrze została przejęta.",
    // 16-20
    L"%s - kopalnia została przejęta. Twój dzienny przychód został zredukowany do %s.",
    L"Nieprzyjaciel bezkonfliktowo przejął sektor %s.",
    L"Przynajmniej jeden z twoich najemników nie został do tego przydzielony.",
    L"%s nie może się przyłączyć, ponieważ %s jest pełny",
    L"%s nie może się przyłączyć, ponieważ %s jest zbyt daleko.",
    // 21-25
    L"%s - kopalnia została przejęta przez siły Deidranny!",
    L"Siły Deidranny właśnie zaatakowały bazę rakiet Ziemia-Powietrze w - %s.",
    L"Siły Deidranny właśnie zaatakowały - %s.",
    L"Właśnie zauważono siły Deidranny w - %s.",
    L"Siły Deidranny właśnie przejęły - %s.",
    // 26-30
    L"Przynajmniej jeden z twoich najemników nie mógł się położyć spać.",
    L"Przynajmniej jeden z twoich najemników nie mógł wstać.",
    L"Oddziały samoobrony nie pojawią się dopóki nie zostaną wyszkolone.",
    L"%s nie może się w tej chwili przemieszczać.",
    L"Żołnierze samoobrony, którzy znajdują się poza granicami miasta, nie mogą być przeniesieni "
    L"do innego sektora.",
    // 31-35
    L"Nie możesz trenować samoobrony w - %s.",
    L"Pusty pojazd nie może się poruszać!",
    L"%s ma zbyt wiele ran by podróżować!",
    L"Musisz wpierw opuścić muzeum!",
    L"%s nie żyje!",
    // 36-40
    L"%s nie może się zamienić z - %s, ponieważ się porusza",
    L"%s nie może w ten sposób wejśc do pojazdu",
    L"%s nie może się dołączyć do - %s",
    L"Nie możesz kompresować czasu dopóki nie zatrudnisz sobie kilku nowych najemników!",
    L"Ten pojazd może się poruszać tylko po drodze!",
    // 41-45
    L"Nie można zmieniać przydziału najemników, którzy są w drodze",
    L"Pojazd nie ma paliwa!",
    L"%s jest zbyt zmęczony(na) by podróżować.",
    L"Żaden z pasażerów nie jest w stanie kierować tym pojazdem.",
    L"Jeden lub więcej członków tego oddziału nie może się w tej chwili przemieszczać.",
    // 46-50
    L"Jeden lub więcej INNYCH członków tego oddziału nie może się w tej chwili przemieszczać.",
    L"Pojazd jest uszkodzony!",
    L"Pamiętaj, że w jednym sektorze tylko dwóch najemników może trenować żołnierzy samoobrony.",
    L"Robot nie może się poruszać bez operatora. Umieść ich razem w jednym oddziale.",
};

// help text used during strategic route plotting
wchar_t* pMapPlotStrings[] = {
    L"Kliknij ponownie sektor docelowy, aby zatwierdzić trasę podróży, lub kliknij inny sektor, "
    L"aby ją wydłużyć.",
    L"Trasa podróży zatwierdzona.",
    L"Cel podróży nie został zmieniony.",
    L"Trasa podróży została anulowana.",
    L"Trasa podróży została skrócona.",
};

// help text used when moving the merc arrival sector
wchar_t* pBullseyeStrings[] = {
    L"Kliknij sektor, do którego mają przylatywać najemnicy.",
    L"Dobrze. Przylatujący najemnicy będą zrzucani w %s",
    L"Najemnicy nie mogą tu przylatywać. Przestrzeń powietrzna nie jest zabezpieczona!",
    L"Anulowano. Sektor zrzutu nie został zmieniony.",
    L"Przestrzeń powietrzna nad %s nie jest już bezpieczna! Sektor zrzutu został przesunięty do "
    L"%s.",
};

// help text for mouse regions

wchar_t* pMiscMapScreenMouseRegionHelpText[] = {
    L"Otwórz wyposażenie (|E|n|t|e|r)",
    L"Zniszcz przedmiot",
    L"Zamknij wyposażenie (|E|n|t|e|r)",
};

// male version of where equipment is left
wchar_t* pMercHeLeaveString[] = {
    L"Czy %s ma zostawić swój sprzęt w sektorze, w którym się obecnie znajduje (%s), czy w Dressen "
    L"(B13), skąd odlatuje? ",
    L"Czy %s ma zostawić swój sprzęt w sektorze, w którym się obecnie znajduje (%s), czy w Omercie "
    L"(A9), skąd odlatuje?",
    L"wkrótce odchodzi i zostawi swój sprzęt w Omercie (A9).",
    L"wkrótce odchodzi i zostawi swój sprzęt w Drassen (B13).",
    L"%s wkrótce odchodzi i zostawi swój sprzęt w %s.",
};

// female version
wchar_t* pMercSheLeaveString[] = {
    L"Czy %s ma zostawić swój sprzęt w sektorze, w którym się obecnie znajduje (%s), czy w Dressen "
    L"(B13), skąd odlatuje? ",
    L"Czy %s ma zostawić swój sprzęt w sektorze, w którym się obecnie znajduje (%s), czy w Omercie "
    L"(A9), skąd odlatuje?",
    L"wkrótce odchodzi i zostawi swój sprzęt w Omercie (A9).",
    L"wkrótce odchodzi i zostawi swój sprzęt w Drassen (B13).",
    L"%s wkrótce odchodzi i zostawi swój sprzęt w %s.",
};

wchar_t* pMercContractOverStrings[] = {
    L" zakończył kontrakt więc wyjechał.",             // merc's contract is over and has departed
    L" zakończyła kontrakt więc wyjechała.",           // merc's contract is over and has departed
    L" - jego kontrakt został zerwany więc odszedł.",  // merc's contract has been terminated
    L" - jej kontrakt został zerwany więc odeszła.",   // merc's contract has been terminated
    L"Masz za duży dług wobec M.E.R.C. więc %s odchodzi.",  // Your M.E.R.C. account is invalid so
                                                            // merc left
};

// Text used on IMP Web Pages

wchar_t* pImpPopUpStrings[] = {
    L"Nieprawidłowy kod dostępu",
    L"Czy na pewno chcesz wznowić proces określenia profilu?",
    L"Wprowadź nazwisko oraz płeć",
    L"Wstępna kontrola stanu twoich finansów wykazała, że nie stać cię na analizę profilu.",
    L"Opcja tym razem nieaktywna.",
    L"Aby wykonać profil, musisz mieć miejsce dla przynajmniej jednego członka załogi.",
    L"Profil został już wykonany.",
};

// button labels used on the IMP site

wchar_t* pImpButtonText[] = {
    L"O Nas",                // about the IMP site
    L"ZACZNIJ",              // begin profiling
    L"Osobowość",            // personality section
    L"Atrybuty",             // personal stats/attributes section
    L"Portret",              // the personal portrait selection
    L"Głos %d",              // the voice selection
    L"Gotowe",               // done profiling
    L"Zacznij od początku",  // start over profiling
    L"Tak, wybieram tą odpowiedź.",
    L"Tak",
    L"Nie",
    L"Skończone",                  // finished answering questions
    L"Poprz.",                     // previous question..abbreviated form
    L"Nast.",                      // next question
    L"TAK, JESTEM.",               // yes, I am certain
    L"NIE, CHCĘ ZACZĄĆ OD NOWA.",  // no, I want to start over the profiling process
    L"TAK",
    L"NIE",
    L"Wstecz",  // back one page
    L"Anuluj",  // cancel selection
    L"Tak.",
    L"Nie, Chcę spojrzeć jeszcze raz.",
    L"Rejestr",       // the IMP site registry..when name and gender is selected
    L"Analizuję...",  // analyzing your profile results
    L"OK",
    L"Głos",
};

wchar_t* pExtraIMPStrings[] = {
    L"Aby zacząć analizę profilu, wybierz osobowość.", L"Teraz określ swoje atrybuty.",
    L"Teraz możesz przystąpić do wyboru portretu.",
    L"Aby zakończyć proces, wybierz próbkę głosu, która ci najbardziej odpowiada."};

wchar_t* pFilesTitle[] = {
    L"Przeglądarka plików",
};

wchar_t* pFilesSenderList[] = {
    L"Raport Rozp.",  // the recon report sent to the player. Recon is an abbreviation for
                      // reconissance
    L"Intercept #1",  // first intercept file .. Intercept is the title of the organization sending
                      // the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in
                      // files.txt for the translated title
    L"Intercept #2",  // second intercept file
    L"Intercept #3",  // third intercept file
    L"Intercept #4",  // fourth intercept file
    L"Intercept #5",  // fifth intercept file
    L"Intercept #6",  // sixth intercept file
};

// Text having to do with the History Log

wchar_t* pHistoryTitle[] = {
    L"Historia",
};

wchar_t* pHistoryHeaders[] = {
    L"Dzień",      // the day the history event occurred
    L"Strona",     // the current page in the history report we are in
    L"Dzień",      // the days the history report occurs over
    L"Położenie",  // location (in sector) the event occurred
    L"Zdarzenie",  // the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
wchar_t* pHistoryStrings[] = {
    L"",  // leave this line blank
    // 1-5
    L"%s najęty(ta) w A.I.M.",           // merc was hired from the aim site
    L"%s najęty(ta) w M.E.R.C.",         // merc was hired from the aim site
    L"%s ginie.",                        // merc was killed
    L"Uregulowano rachunki w M.E.R.C.",  // paid outstanding bills at MERC
    L"Przyjęto zlecenie od Enrico Chivaldori",
    // 6-10
    L"Profil IMP wygenerowany",
    L"Podpisano umowę ubezpieczeniową dla %s.",  // insurance contract purchased
    L"Anulowano umowę ubezpieczeniową dla %s.",  // insurance contract canceled
    L"Wypłata ubezpieczenia za %s.",             // insurance claim payout for merc
    L"Przedłużono kontrakt z: %s o 1 dzień.",    // Extented "mercs name"'s for a day
    // 11-15
    L"Przedłużono kontrakt z: %s o 1 tydzień.",   // Extented "mercs name"'s for a week
    L"Przedłużono kontrakt z: %s o 2 tygodnie.",  // Extented "mercs name"'s 2 weeks
    L"%s zwolniony(na).",                         // "merc's name" was dismissed.
    L"%s odchodzi.",                              // "merc's name" quit.
    L"przyjęto zadanie.",                         // a particular quest started
    // 16-20
    L"zadanie wykonane.",
    L"Rozmawiano szefem kopalni %s",  // talked to head miner of town
    L"Wyzwolono - %s",
    L"Użyto kodu Cheat",
    L"Żywność powinna być jutro w Omercie",
    // 21-25
    L"%s odchodzi, aby wziąć ślub z Darylem Hickiem",
    L"Wygasł kontrakt z - %s.",
    L"%s zrekrutowany(na).",
    L"Enrico narzeka na brak postępów",
    L"Walka wygrana",
    // 26-30
    L"%s - w kopalni kończy się ruda",
    L"%s - w kopalni skończyła się ruda",
    L"%s - kopalnia została zamknięta",
    L"%s - kopalnia została otwarta",
    L"Informacja o więzieniu zwanym Tixa.",
    // 31-35
    L"Informacja o tajnej fabryce broni zwanej Orta.",
    L"Naukowiec w Orcie ofiarował kilka karabinów rakietowych.",
    L"Królowa Deidranna robi użytek ze zwłok.",
    L"Frank opowiedział o walkach w San Monie.",
    L"Pewien pacjent twierdzi, że widział coś w kopalni.",
    // 36-40
    L"Gość o imieniu Devin sprzedaje materiały wybuchowe.",
    L"Spotkanie ze sławynm eks-najemnikiem A.I.M. - Mike'iem!",
    L"Tony handluje bronią.",
    L"Otrzymano karabin rakietowy od sierżanta Krotta.",
    L"Dano Kyle'owi akt własności sklepu Angela.",
    // 41-45
    L"Madlab zaoferował się zbudować robota.",
    L"Gabby potrafi zrobić miksturę chroniącą przed robakami.",
    L"Keith wypadł z interesu.",
    L"Howard dostarczał cyjanek królowej Deidrannie.",
    L"Spotkanie z handlarzem Keithem w Cambrii.",
    // 46-50
    L"Spotkanie z aptekarzem Howardem w Balime",
    L"Spotkanie z Perko, prowadzącym mały warsztat.",
    L"Spotkanie z Samem z Balime - prowadzi sklep z narzędziami.",
    L"Franz handluje sprzętem elektronicznym.",
    L"Arnold prowadzi warsztat w Grumm.",
    // 51-55
    L"Fredo naprawia sprzęt elektroniczny w Grumm.",
    L"Otrzymano darowiznę od bogatego gościa w Balime.",
    L"Spotkano Jake'a, który prowadzi złomowisko.",
    L"Jakiś włóczęga dał nam elektroniczną kartę dostępu.",
    L"Przekupiono Waltera, aby otworzył drzwi do piwnicy.",
    // 56-60
    L"Dave oferuje darmowe tankowania, jeśli będzie miał paliwo.",
    L"Greased Pablo's palms.",
    L"Kingpin trzyma pieniądze w kopalni w San Mona.",
    L"%s wygrał(a) walkę",
    L"%s przegrał(a) walkę",
    // 61-65
    L"%s zdyskwalifikowany(na) podczas walki",
    L"Znaleziono dużo pieniędzy w opuszczonej kopalni.",
    L"Spotkano zabójcę nasłanego przez Kingpina.",
    L"Utrata kontroli nad sektorem",  // ENEMY_INVASION_CODE
    L"Sektor obroniony",
    // 66-70
    L"Przegrana bitwa",   // ENEMY_ENCOUNTER_CODE
    L"Fatalna zasadzka",  // ENEMY_AMBUSH_CODE
    L"Usunieto zasadzkę wroga",
    L"Nieudany atak",  // ENTERING_ENEMY_SECTOR_CODE
    L"Udany atak!",
    // 71-75
    L"Stworzenia zaatakowały",        // CREATURE_ATTACK_CODE
    L"Zabity(ta) przez dzikie koty",  // BLOODCAT_AMBUSH_CODE
    L"Wyrżnięto dzikie koty",
    L"%s zabity(ta)",
    L"Przekazano Carmenowi głowę terrorysty",
    L"Slay odszedł",
    L"Zabito: %s",
};

wchar_t* pHistoryLocations[] = {
    L"N/D",  // N/A is an acronym for Not Applicable
};

// icon text strings that appear on the laptop

wchar_t* pLaptopIcons[] = {
    L"E-mail",      L"Sieć", L"Finanse", L"Personel", L"Historia", L"Pliki", L"Zamknij",
    L"sir-FER 4.0",  // our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

wchar_t* pBookMarkStrings[] = {
    L"A.I.M.",   L"Bobby Ray's", L"I.M.P",         L"M.E.R.C.",
    L"Pogrzeby", L"Kwiaty",      L"Ubezpieczenia", L"Anuluj",
};

wchar_t* pBookmarkTitle[] = {
    L"Ulubione",
    L"Aby w przyszłości otworzyć to menu, kliknij prawym klawiszem myszy.",
};

// When loading or download a web page

wchar_t* pDownloadString[] = {
    L"Ładowanie strony...",
    L"Otwieranie strony...",
};

// This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

wchar_t* gsAtmSideButtonText[] = {
    L"OK",
    L"Weź",     // take money from merc
    L"Daj",     // give money to merc
    L"Anuluj",  // cancel transaction
    L"Skasuj",  // clear amount being displayed on the screen
};

wchar_t* gsAtmStartButtonText[] = {
    L"Transfer $",   // transfer money to merc -- short form
    L"Atrybuty",     // view stats of the merc
    L"Wyposażenie",  // view the inventory of the merc
    L"Zatrudnienie",
};

wchar_t* sATMText[] = {
    L"Przesłać fundusze?",                  // transfer funds to merc?
    L"OK?",                                 // are we certain?
    L"Wprowadź kwotę",                      // enter the amount you want to transfer to merc
    L"Wybierz typ",                         // select the type of transfer to merc
    L"Brak środków",                        // not enough money to transfer to merc
    L"Kwota musi być podzielna przez $10",  // transfer amount must be a multiple of $10
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

wchar_t* pErrorStrings[] = {
    L"Błąd",
    L"Serwer nie posiada DNS.",
    L"Sprawdź adres URL i spróbuj ponownie.",
    L"OK",
    L"Niestabilne połączenie z Hostem. Transfer może trwać dłużej.",
};

wchar_t* pPersonnelString[] = {
    L"Najemnicy:",  // mercs we have
};

wchar_t* pWebTitle[] = {
    L"sir-FER 4.0",  // our name for thL"sir-FER 4.0",		// our name for the version of the
                     // browser, play on company name
};

// The titles for the web program title bar, for each page loaded

wchar_t* pWebPagesTitles[] = {
    L"A.I.M.",
    L"A.I.M. Członkowie",
    L"A.I.M. Portrety",  // a mug shot is another name for a portrait
    L"A.I.M. Lista",
    L"A.I.M.",
    L"A.I.M. Weterani",
    L"A.I.M. Polisy",
    L"A.I.M. Historia",
    L"A.I.M. Linki",
    L"M.E.R.C.",
    L"M.E.R.C. Konta",
    L"M.E.R.C. Rejestracja",
    L"M.E.R.C. Indeks",
    L"Bobby Ray's",
    L"Bobby Ray's - Broń",
    L"Bobby Ray's - Amunicja",
    L"Bobby Ray's - Pancerz",
    L"Bobby Ray's - Różne",  // misc is an abbreviation for miscellaneous
    L"Bobby Ray's - Używane",
    L"Bobby Ray's - Zamówienie pocztowe",
    L"I.M.P.",
    L"I.M.P.",
    L"United Floral Service",
    L"United Floral Service - Galeria",
    L"United Floral Service - Zamówienie",
    L"United Floral Service - Galeria kartek",
    L"Malleus, Incus & Stapes - Brokerzy ubezpieczeniowi",
    L"Informacja",
    L"Kontrakt",
    L"Uwagi",
    L"McGillicutty - Zakład pogrzebowy",
    L"",
    L"Nie odnaleziono URL.",
    L"Bobby Ray's - Ostatnie dostawy",
    L"",
    L"",
};

wchar_t* pShowBookmarkString[] = {
    L"Sir-Pomoc",
    L"Kliknij ponownie Sieć by otworzyć menu Ulubione.",
};

wchar_t* pLaptopTitles[] = {
    L"Poczta", L"Przeglądarka plików", L"Personel", L"Księgowy Plus", L"Historia",
};

wchar_t* pPersonnelDepartedStateStrings[] = {
    // reasons why a merc has left.
    L"Śmierć w akcji", L"Zwolnienie", L"Inny", L"Małżeństwo", L"Koniec kontraktu", L"Rezygnacja",
};
// personnel strings appearing in the Personnel Manager on the laptop

wchar_t* pPersonelTeamStrings[] = {
    L"Bieżący oddział",  L"Wyjazdy",         L"Koszt dzienny:", L"Najwyższy koszt:",
    L"Najniższy koszt:", L"Śmierć w akcji:", L"Zwolnienie:",    L"Inny:",
};

wchar_t* pPersonnelCurrentTeamStatsStrings[] = {
    L"Najniższy",
    L"Średni",
    L"Najwyższy",
};

wchar_t* pPersonnelTeamStatsStrings[] = {
    L"ZDR", L"ZWN", L"ZRCZ", L"SIŁA", L"DOW", L"INT", L"DOŚW", L"STRZ", L"MECH", L"WYB", L"MED",
};

// horizontal and vertical indices on the map screen

wchar_t* pMapVertIndex[] = {
    L"X", L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H",
    L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P",
};

wchar_t* pMapHortIndex[] = {
    L"X", L"1",  L"2",  L"3",  L"4",  L"5",  L"6",  L"7",  L"8",
    L"9", L"10", L"11", L"12", L"13", L"14", L"15", L"16",
};

wchar_t* pMapDepthIndex[] = {
    L"",
    L"-1",
    L"-2",
    L"-3",
};

// text that appears on the contract button

wchar_t* pContractButtonString[] = {
    L"Kontrakt",
};

// text that appears on the update panel buttons

wchar_t* pUpdatePanelButtons[] = {
    L"Dalej",
    L"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

uint16_t LargeTacticalStr[][LARGE_STRING_LENGTH] = {
    L"Pokonano cię w tym sektorze!",
    L"Wróg nie zna litości i pożera was wszystkich!",
    L"Nieprzytomni członkowie twojego oddziału zostali pojmani!",
    L"Członkowie twojego oddziału zostali uwięzieni.",
};

// Insurance Contract.c
// The text on the buttons at the bottom of the screen.

wchar_t* InsContractText[] = {
    L"Wstecz",
    L"Dalej",
    // L"Akceptuję",
    L"OK",
    L"Skasuj",
};

// Insurance Info
// Text on the buttons on the bottom of the screen

wchar_t* InsInfoText[] = {L"Wstecz", L"Dalej"};

// For use at the M.E.R.C. web site. Text relating to the player's account with MERC

wchar_t* MercAccountText[] = {
    // Text on the buttons on the bottom of the screen
    L"Autoryzacja",
    L"Strona główna",
    L"Konto #:",
    L"Najemnik",
    L"Dni",
    L"Stawka",  // 5
    L"Opłata",
    L"Razem:",
    L"Czy na pewno chcesz zatwierdzić płatność: %s?",  // the %s is a string that contains the
                                                       // dollar amount ( ex. "$150" )
};

// For use at the M.E.R.C. web site. Text relating a MERC mercenary

wchar_t* MercInfo[] = {
    L"Zdrowie",
    L"Zwinność",
    L"Sprawność",
    L"Siła",
    L"Um. dowodz.",
    L"Inteligencja",
    L"Poz. doświadczenia",
    L"Um. strzeleckie",
    L"Zn. mechaniki",
    L"Mat. wybuchowe",
    L"Wiedza medyczna",

    L"Poprzedni",
    L"Najmij",
    L"Następny",
    L"Dodatkowe informacje",
    L"Strona główna",
    L"Najęty",
    L"Koszt:",
    L"Dziennie",
    L"Nie żyje",

    L"Wygląda na to, że chcesz wynająć zbyt wielu najemników. Limit wynosi 18.",
    L"Niedostępny",
};

// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

wchar_t* MercNoAccountText[] = {
    // Text on the buttons at the bottom of the screen
    L"Otwórz konto", L"Anuluj", L"Nie posiadasz konta. Czy chcesz sobie założyć?"};

// For use at the M.E.R.C. web site. MERC Homepage

wchar_t* MercHomePageText[] = {
    // Description of various parts on the MERC page
    L"Speck T. Kline, założyciel i właściciel",
    L"Aby otworzyć konto naciśnij tu",
    L"Aby zobaczyć konto naciśnij tu",
    L"Aby obejrzeć akta naciśnij tu",
    // The version number on the video conferencing system that pops up when Speck is talking
    L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

wchar_t* sFuneralString[] = {
    L"Zakład pogrzebowy McGillicutty, pomaga rodzinom pogrążonym w smutku od 1983.",
    L"Kierownik, były najemnik A.I.M. Murray \'Pops\' McGillicutty jest doświadczonym pracownikiem "
    L"zakładu pogrzebowego.",
    L"Przez całe życie obcował ze śmiercią, 'Pops' wie jak trudne są te chwile.",
    L"Zakład pogrzebowy McGillicutty oferuje szeroki zakres usług, od duchowego wsparcia po "
    L"rekonstrukcję silnie zniekształconych zwłok.",
    L"Pozwól by McGillicutty ci pomógł a twój ukochany będzie spoczywał w pokoju.",

    // Text for the various links available at the bottom of the page
    L"WYŚLIJ KWIATY", L"KOLEKCJA TRUMIEN I URN", L"USŁUGI KREMA- CYJNE",
    L"USŁUGI PLANOWANIA POGRZEBU", L"KARTKI POGRZE- BOWE",

    // The text that comes up when you click on any of the links ( except for send flowers ).
    L"Niestety, z powodu śmierci w rodzinie, nie działają jeszcze wszystkie elementy tej strony.",
    L"Przepraszamy za powyższe uniedogodnienie."};

// Text for the florist Home page

wchar_t* sFloristText[] = {
    // Text on the button on the bottom of the page

    L"Galeria",

    // Address of United Florist

    L"\"Zrzucamy z samolotu w dowolnym miejscu\"", L"1-555-POCZUJ-MNIE",
    L"Ul. Nosowska 333, Zapuszczone miasto, CA USA 90210", L"http://www.poczuj-mnie.com",

    // detail of the florist page

    L"Działamy szybko i sprawnie!",
    L"Gwarantujemy dostawę w dowolny punkt na Ziemi, następnego dnia po złożeniu zamówienia!",
    L"Oferujemy najniższe ceny na świecie!",
    L"Pokaż nam ofertę z niższą ceną, a dostaniesz w nagrodę tuzin róż, za darmo!",
    L"Latająca flora, fauna i kwiaty od 1981.",
    L"Nasz ozdobiony bombowiec zrzuci twój bukiet w promieniu co najwyżej dziesięciu mil od "
    L"żądanego miejsca. Kiedy tylko zechcesz!",
    L"Pozwól nam zaspokoić twoje kwieciste fantazje.",
    L"Bruce, nasz światowej renomy projektant bukietów, zerwie dla ciebie najświeższe i "
    L"najwspanialsze kwiaty z naszej szklarni.",
    L"I pamiętaj, jeśli czegoś nie mamy, możemy to szybko zasadzić!"};

// Florist OrderForm

wchar_t* sOrderFormText[] = {
    // Text on the buttons

    L"Powrót",
    L"Wyślij",
    L"Skasuj",
    L"Galeria",

    L"Nazwa bukietu:",
    L"Cena:",  // 5
    L"Zamówienie numer:",
    L"Czas dostawy",
    L"nast. dnia",
    L"dostawa gdy to będzie możliwe",
    L"Miejsce dostawy",  // 10
    L"Dodatkowe usługi",
    L"Zgnieciony bukiet($10)",
    L"Czarne Róże($20)",
    L"Zwiędnięty bukiet($10)",
    L"Ciasto owocowe (jeżeli będzie)($10)",  // 15
    L"Osobiste kondolencje:",
    L"Ze względu na rozmiar karteczek, tekst nie może zawierać więcej niż 75 znaków.",
    L"...możesz też przejrzeć nasze",

    L"STANDARDOWE KARTKI",
    L"Informacja o rachunku",  // 20

    // The text that goes beside the area where the user can enter their name

    L"Nazwisko:",
};

// Florist Gallery.c

wchar_t* sFloristGalleryText[] = {
    // text on the buttons

    L"Poprz.",  // abbreviation for previous
    L"Nast.",   // abbreviation for next

    L"Kliknij wybraną pozycję aby złożyć zamówienie.",
    L"Uwaga: $10 dodatkowej opłaty za zwiędnięty lub zgnieciony bukiet.",

    // text on the button

    L"Główna",
};

// Florist Cards

wchar_t* sFloristCards[] = {L"Kliknij swój wybór", L"Wstecz"};

// Text for Bobby Ray's Mail Order Site

wchar_t* BobbyROrderFormText[] = {
    L"Formularz zamówienia",  // Title of the page
    L"Ilość",                 // The number of items ordered
    L"Waga (%s)",             // The weight of the item
    L"Nazwa",                 // The name of the item
    L"Cena",                  // the item's weight
    L"Wartość",               // 5	// The total price of all of items of the same type
    L"W sumie",               // The sub total of all the item totals added
    L"Transport",             // S&H is an acronym for Shipping and Handling
    L"Razem",                 // The grand total of all item totals + the shipping and handling
    L"Miejsce dostawy",
    L"Czas dostawy",              // 10	// See below
    L"Koszt (za %s.)",            // The cost to ship the items
    L"Ekspres - 24h",             // Gets deliverd the next day
    L"2 dni robocze",             // Gets delivered in 2 days
    L"Standardowa dostawa",       // Gets delivered in 3 days
    L" Wyczyść",                  // 15			// Clears the order page
    L" Akceptuję",                // Accept the order
    L"Wstecz",                    // text on the button that returns to the previous page
    L"Strona główna",             // Text on the button that returns to the home page
    L"* oznacza używane rzeczy",  // Disclaimer stating that the item is used
    L"Nie stać cię na to.",       // 20	// A popup message that to warn of not enough money
    L"<BRAK>",                    // Gets displayed when there is no valid city selected
    L"Miejsce docelowe przesyłki: %s. Potwierdzasz?",  // A popup that asks if the city selected is
                                                       // the correct one
    L"Waga przesyłki*",                                // Displays the weight of the package
    L"* Min. Waga",  // Disclaimer states that there is a minimum weight for the package
    L"Dostawy",
};

// This text is used when on the various Bobby Ray Web site pages that sell items

wchar_t* BobbyRText[] = {
    L"Zamów",  // Title

    L"Kliknij wybrane towary. Lewym klawiszem zwiększasz ilość towaru, a prawym zmniejszasz. Gdy "
    L"już skompletujesz swoje zakupy przejdź do formularza zamówienia.",  // instructions on how to
                                                                          // order

    // Text on the buttons to go the various links

    L"Poprzednia",   //
    L"Broń",         // 3
    L"Amunicja",     // 4
    L"Ochraniacze",  // 5
    L"Różne",        // 6	//misc is an abbreviation for miscellaneous
    L"Używane",      // 7
    L"Następna",
    L"FORMULARZ",
    L"Strona główna",  // 10

    // The following 2 lines are used on the Ammunition page.
    // They are used for help text to display how many items the player's merc has
    // that can use this type of ammo

    L"Twój zespół posiada",                             // 11
    L"szt. broni do której pasuje amunicja tego typu",  // 12

    // The following lines provide information on the items

    L"Waga:",             // Weight of all the items of the same type
    L"Kal:",              // the caliber of the gun
    L"Mag:",              // number of rounds of ammo the Magazine can hold
    L"Zas:",              // The range of the gun
    L"Siła:",             // Damage of the weapon
    L"CS:",               // Weapon's Rate Of Fire, acroymn ROF
    L"Koszt:",            // Cost of the item
    L"Na stanie:",        // The number of items still in the store's inventory
    L"Ilość na zamów.:",  // The number of items on order
    L"Uszkodz.",          // If the item is damaged
    L"Waga:",             // the Weight of the item
    L"Razem:",            // The total cost of all items on order
    L"* Stan: %%",        // if the item is damaged, displays the percent function of the item

    // Popup that tells the player that they can only order 10 items at a time

    L"Przepraszamy za to utrudnienie, ale na jednym zamówieniu może się znajdować tylko 10 "
    L"pozycji! Jeśli potrzebujesz więcej, złóż kolejne zamówienie.",

    // A popup that tells the user that they are trying to order more items then the store has in
    // stock

    L"Przykro nam. Chwilowo nie mamy tego więcej na magazynie. Proszę spróbować później.",

    // A popup that tells the user that the store is temporarily sold out

    L"Przykro nam, ale chwilowo nie mamy tego towaru na magazynie",

};

// Text for Bobby Ray's Home Page

wchar_t* BobbyRaysFrontText[] = {
    // Details on the web site

    L"Tu znajdziesz nowości z dziedziny broni i osprzętu wojskowego",
    L"Zaspokoimy wszystkie twoje potrzeby w dziedzinie materiałów wybuchowych",
    L"UŻYWANE RZECZY",

    // Text for the various links to the sub pages

    L"RÓŻNE",
    L"BROŃ",
    L"AMUNICJA",  // 5
    L"OCHRANIACZE",

    // Details on the web site

    L"Jeśli MY tego nie mamy, to znaczy, że nigdzie tego nie dostaniesz!",
    L"W trakcie budowy",
};

// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug
// shot page

wchar_t* AimSortText[] = {
    L"Członkowie A.I.M.",  // Title

    L"Sortuj wg:",  // Title for the way to sort

    // sort by...

    L"Ceny",
    L"Doświadczenia",
    L"Um. strzeleckich",
    L"Um. med.",
    L"Zn. mat. wyb.",
    L"Zn. mechaniki",

    // Text of the links to other AIM pages

    L"Portrety najemników",
    L"Akta najemnika",
    L"Pokaż galerię byłych członków A.I.M.",

    // text to display how the entries will be sorted

    L"Rosnąco",
    L"Malejąco",
};

// Aim Policies.c
// The page in which the AIM policies and regulations are displayed

wchar_t* AimPolicyText[] = {
    // The text on the buttons at the bottom of the page

    L"Poprzednia str.", L"Strona główna", L"Przepisy", L"Następna str.", L"Rezygnuję", L"Akceptuję",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot
// index

wchar_t* AimMemberText[] = {
    L"Lewy klawisz myszy",
    L"kontakt z najemnikiem",
    L"Prawy klawisz myszy",
    L"lista portretów",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

wchar_t* CharacterInfo[] = {
    // The various attributes of the merc

    L"Zdrowie", L"Zwinność", L"Sprawność", L"Siła", L"Um. dowodzenia", L"Inteligencja",
    L"Poziom dośw.", L"Um. strzeleckie", L"Zn. mechaniki", L"Zn. mat. wyb.",
    L"Wiedza med.",  // 10

    // the contract expenses' area

    L"Zapłata", L"Czas", L"1 dzień", L"1 tydzień", L"2 tygodnie",

    // text for the buttons that either go to the previous merc,
    // start talking to the merc, or go to the next merc

    L"Poprzedni", L"Kontakt", L"Następny",

    L"Dodatkowe informacje",           // Title for the additional info for the merc's bio
    L"Aktywni członkowie",             // 20		// Title of the page
    L"Opcjonalne wyposażenie:",        // Displays the optional gear cost
    L"Wymagany jest zastaw na życie",  // If the merc required a medical deposit, this is displayed
};

// Aim Member.c
// The page in which the player's hires AIM mercenaries

// The following text is used with the video conference popup

wchar_t* VideoConfercingText[] = {
    L"Wartość kontraktu:",  // Title beside the cost of hiring the merc

    // Text on the buttons to select the length of time the merc can be hired

    L"Jeden dzień", L"Jeden tydzień", L"Dwa tygodnie",

    // Text on the buttons to determine if you want the merc to come with the equipment

    L"Bez sprzętu", L"Weź sprzęt",

    // Text on the Buttons

    L"TRANSFER",  // to actually hire the merc
    L"ANULUJ",    // go back to the previous menu
    L"WYNAJMIJ",  // go to menu in which you can hire the merc
    L"ROZŁĄCZ",   // stops talking with the merc
    L"OK",
    L"NAGRAJ SIĘ",  // if the merc is not there, you can leave a message

    // Text on the top of the video conference popup

    L"Wideo konferencja z - ", L"Łączę. . .",

    L"z zastawem"  // Displays if you are hiring the merc with the medical deposit
};

// Aim Member.c
// The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

wchar_t* AimPopUpText[] = {
    L"TRANSFER ZAKOŃCZONY POMYŚLNIE",          // You hired the merc
    L"PRZEPROWADZENIE TRANSFERU NIE MOŻLIWE",  // Player doesn't have enough money, message 1
    L"BRAK ŚRODKÓW",                           // Player doesn't have enough money, message 2

    // if the merc is not available, one of the following is displayed over the merc's face

    L"Wynajęto",
    L"Proszę zostaw wiadomość",
    L"Nie żyje",

    // If you try to hire more mercs than game can support

    L"Masz już pełny zespół 18 najemników.",

    L"Nagrana wiadomość",
    L"Wiadomość zapisana",
};

// AIM Link.c

wchar_t* AimLinkText[] = {
    L"A.I.M. Linki",  // The title of the AIM links page
};

// Aim History

// This page displays the history of AIM

wchar_t* AimHistoryText[] = {L"A.I.M. Historia",  // Title

                             // Text on the buttons at the bottom of the page

                             L"Poprzednia str.", L"Strona główna", L"Byli członkowie",
                             L"Następna str."};

// Aim Mug Shot Index

// The page in which all the AIM members' portraits are displayed in the order selected by the AIM
// sort page.

wchar_t* AimFiText[] = {
    // displays the way in which the mercs were sorted

    L"ceny",
    L"doświadczenia",
    L"um. strzeleckich",
    L"um. medycznych",
    L"zn. materiałów wyb.",
    L"zn. mechaniki",

    // The title of the page, the above text gets added at the end of this text

    L"Członkowie A.I.M. posortowani rosnąco wg %s",
    L"Członkowie A.I.M. posortowani malejąco wg %s",

    // Instructions to the players on what to do

    L"Lewy klawisz",
    L"Wybór najemnika",  // 10
    L"Prawy klawisz",
    L"Opcje sortowania",

    // Gets displayed on top of the merc's portrait if they are...

    L"Wyjechał(a)",
    L"Nie żyje",  // 14
    L"Wynajęto",
};

// AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer
// with AIM

wchar_t* AimAlumniText[] = {

    L"STRONA 1", L"STRONA 2", L"STRONA 3",

    L"Byli członkowie A.I.M.",  // Title of the page

    L"OK"  // Stops displaying information on selected merc
};

// AIM Home Page

wchar_t* AimScreenText[] = {
    // AIM disclaimers

    L"Znaki A.I.M. i logo A.I.M. są prawnie chronione w większości krajów.",
    L"Więc nawet nie myśl o próbie ich podrobienia.",
    L"Copyright 1998-1999 A.I.M., Ltd. All rights reserved.",

    // Text for an advertisement that gets displayed on the AIM page

    L"United Floral Service",
    L"\"Zrzucamy gdziekolwiek\"",  // 10
    L"Zrób to jak należy...",
    L"...za pierwszym razem",
    L"Broń i akcesoria, jeśli czegoś nie mamy, to tego nie potrzebujesz.",
};

// Aim Home Page

wchar_t* AimBottomMenuText[] = {
    // Text for the links at the bottom of all AIM pages
    L"Strona główna", L"Członkowie", L"Byli członkowie", L"Przepisy", L"Historia", L"Linki",
};

// ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

wchar_t* SKI_Text[] = {
    L"TOWARY NA STANIE",  // Header for the merchandise available
    L"STRONA",            // The current store inventory page being displayed
    L"KOSZT OGÓŁEM",      // The total cost of the the items in the Dealer inventory area
    L"WARTOŚĆ OGÓŁEM",    // The total value of items player wishes to sell
    L"WYCENA",            // Button text for dealer to evaluate items the player wants to sell
    L"TRANSAKCJA",        // Button text which completes the deal. Makes the transaction.
    L"OK",                // Text for the button which will leave the shopkeeper interface.
    L"KOSZT NAPRAWY",     // The amount the dealer will charge to repair the merc's goods
    L"1 GODZINA",  // SINGULAR! The text underneath the inventory slot when an item is given to the
                   // dealer to be repaired
    L"%d GODZIN(Y)",  // PLURAL!   The text underneath the inventory slot when an item is given to
                      // the dealer to be repaired
    L"NAPRAWIONO",    // Text appearing over an item that has just been repaired by a NPC repairman
                      // dealer
    L"Brak miejsca by zaoferować więcej rzeczy.",  // Message box that tells the user there is no
                                                   // more room to put there stuff
    L"%d MINUT(Y)",  // The text underneath the inventory slot when an item is given to the dealer
                     // to be repaired
    L"Upuść przedmiot na ziemię.",
};

// ShopKeeper Interface
// for the bank machine panels. Referenced here is the acronym ATM, which means Automatic Teller
// Machine

wchar_t* SkiAtmText[] = {
    // Text on buttons on the banking machine, displayed at the bottom of the page
    L"0",      L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9",
    L"OK",      // Transfer the money
    L"Weź",     // Take money from the player
    L"Daj",     // Give money to the player
    L"Anuluj",  // Cancel the transfer
    L"Skasuj",  // Clear the money display
};

// Shopkeeper Interface
wchar_t* gzSkiAtmText[] = {

    // Text on the bank machine panel that....
    L"Wybierz",         // tells the user to select either to give or take from the merc
    L"Wprowadź kwotę",  // Enter the amount to transfer
    L"Transfer gotówki do najemnika",  // Giving money to the merc
    L"Transfer gotówki od najemnika",  // Taking money from the merc
    L"Brak środków",                   // Not enough money to transfer
    L"Saldo",                          // Display the amount of money the player currently has
};

wchar_t* SkiMessageBoxText[] = {
    L"Czy chcesz dołożyć %s ze swojego konta, aby pokryć różnicę?",
    L"Brak środków. Brakuje ci %s",
    L"Czy chcesz przeznaczyć %s ze swojego konta, aby pokryć koszty?",
    L"Poproś o rozpoczęcie transakscji",
    L"Poproś o naprawę wybranych przedmiotów",
    L"Zakończ rozmowę",
    L"Saldo dostępne",
};

// OptionScreen.c

wchar_t* zOptionsText[] = {
    // button Text
    L"Zapisz grę",
    L"Odczytaj grę",
    L"Wyjście",
    L"OK",

    // Text above the slider bars
    L"Efekty",
    L"Dialogi",
    L"Muzyka",

    // Confirmation pop when the user selects..
    L"Zakończyć grę i wrócić do głównego menu?",

    L"Musisz włączyć opcję dialogów lub napisów.",
};

// SaveLoadScreen
wchar_t* zSaveLoadText[] = {
    L"Zapisz grę",
    L"Odczytaj grę",
    L"Anuluj",
    L"Zapisz wybraną",
    L"Odczytaj wybraną",

    L"Gra została pomyślnie zapisana",
    L"BŁĄD podczas zapisu gry!",
    L"Gra została pomyślnie odczytana",
    L"BŁĄD podczas odczytu gry!",

    L"Wersja gry w zapisanym pliku różni się od bieżącej. Prawdopodobnie można bezpiecznie "
    L"kontynuować. Kontynuować?",
    L"Zapisane pliki gier mogą być uszkodzone. Czy chcesz je usunąć?",

// Translators, the next two strings are for the same thing.  The first one is for beta version
// releases and the second one is used for the final version.  Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"Nieprawidłowa wersja zapisu gry.  W razie problemów prosimy o raport.  Kontynuować?",
#else
    L"Próba odczytu starszej wersji zapisu gry.  Zaktualizować ten zapis i odczytać grę?",
#endif

// Translators, the next two strings are for the same thing.  The first one is for beta version
// releases and the second one is used for the final version.  Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"Nieprawidłowa wersja zapisu gry.  W razie problemów prosimy o raport.  Kontynuować?",
#else
    L"Próba odczytu starszej wersji zapisu gry.  Zaktualizować ten zapis i odczytać grę?",
#endif

    L"Czy na pewno chcesz nadpisać grę na pozycji %d?",
    L"Chcesz odczytać grę z pozycji",

    // The first %d is a number that contains the amount of free space on the users hard drive,
    // the second is the recommended amount of free space.
    L"Brak miejsca na dysku twardym.  Na dysku wolne jest %d MB, a wymagane jest przynajmniej %d "
    L"MB.",

    L"Zapisuję...",  // When saving a game, a message box with this string appears on the screen

    L"Standardowe uzbrojenie",
    L"Całe mnóstwo broni",
    L"Realistyczna gra",
    L"Elementy S-F",

    L"Stopień trudności",
};

// MapScreen
wchar_t* zMarksMapScreenText[] = {
    L"Poziom mapy",
    L"Nie masz jeszcze żołnierzy samoobrony.  Musisz najpierw wytrenować mieszkańców miast.",
    L"Dzienny przychód",
    L"Najmemnik ma polisę ubezpieczeniową",
    L"%s nie potrzebuje snu.",
    L"%s jest w drodze i nie może spać",
    L"%s jest zbyt zmęczony(na), spróbuj trochę później.",
    L"%s prowadzi.",
    L"Oddział nie może się poruszać jeżeli jeden z najemników śpi.",

    // stuff for contracts
    L"Mimo, że możesz opłacić kontrakt, to jednak nie masz gotówki by opłacić składkę "
    L"ubezpieczeniową za najemnika.",
    L"%s - składka ubezpieczeniowa najemnika będzie kosztować %s za %d dzień(dni). Czy chcesz ją "
    L"opłacić?",
    L"Inwentarz sektora",
    L"Najemnik posiada zastaw na życie.",

    // other items
    L"Lekarze",   // people acting a field medics and bandaging wounded mercs //
                  // **************************************NEW******** as of July 09, 1998
    L"Pacjenci",  // people who are being bandaged by a medic //
                  // ****************************************************NEW******** as of July 10,
                  // 1998
    L"Gotowe",    // Continue on with the game after autobandage is complete
    L"Przerwij",  // Stop autobandaging of patients by medics now
    L"Przykro nam, ale ta opcja jest wyłączona w wersji demo.",  // informs player this
                                                                 // option/button has been disabled
                                                                 // in the demo
    L"%s nie ma zestawu narzędzi.",
    L"%s nie ma apteczki.",
    L"Brak chętnych ludzi do szkolenia, w tej chwili.",
    L"%s posiada już maksymalną liczbę oddziałów samoobrony.",
    L"Najemnik ma kontrakt na określony czas.",
    L"Kontrakt najemnika nie jest ubezpieczony",
};

wchar_t* pLandMarkInSectorString[] = {
    L"Oddział %d zauważył kogoś w sektorze %s",
};

// confirm the player wants to pay X dollars to build a militia force in town
wchar_t* pMilitiaConfirmStrings[] = {
    L"Szkolenie oddziału samoobrony będzie kosztowało $",  // telling player how much it will cost
    L"Zatwierdzasz wydatek?",  // asking player if they wish to pay the amount requested
    L"Nie stać cię na to.",    // telling the player they can't afford to train this town
    L"Kontynuować szkolenie samoobrony w - %s (%s %d)?",  // continue training this town?
    L"Koszt $",                                           // the cost in dollars to train militia
    L"( T/N )",                                           // abbreviated yes/no
    L"",                                                  // unused
    L"Szkolenie samoobrony w %d sektorach będzie kosztowało $ %d. %s",  // cost to train sveral
                                                                        // sectors at once
    L"Nie masz %d$, aby wyszkolić samoobronę w tym mieście.",
    L"%s musi mieć %d% lojalności, aby można było kontynuować szkolenie samoobrony.",
    L"Nie możesz już dłużej szkolić samoobrony w mieście %s.",
};

// Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom
// of the single merc panel
wchar_t* gzMoneyWithdrawMessageText[] = {
    L"Jednorazowo możesz wypłacić do 20,000$.",
    L"Czy na pewno chcesz wpłacić %s na swoje konto?",
};

wchar_t* gzCopyrightText[] = {
    L"Copyright (C) 1999 Sir-tech Canada Ltd.  All rights reserved.",
};

// option Text
wchar_t* zOptionsToggleText[] = {
    L"Dialogi",
    L"Wycisz potwierdzenia",
    L"Napisy",
    L"Wstrzymuj napisy",
    L"Animowany dym",
    L"Drastyczne sceny",
    L"Nigdy nie ruszaj mojej myszki!",
    L"Stara metoda wyboru",
    L"Pokazuj trasę ruchu",
    L"Pokazuj chybione strzały",
    L"Potwierdzenia w trybie Real-Time",
    L"Informacja, że najemnik śpi/budzi się",
    L"Używaj systemu metrycznego",
    L"Światło wokół najemników podczas ruchu",
    L"Przyciągaj kursor do najemników",
    L"Przyciągaj kursor do drzwi",
    L"Pulsujące przedmioty",
    L"Pokazuj korony drzew",
    L"Pokazuj siatkę",
    L"Pokazuj kursor 3D",
};

// This is the help text associated with the above toggles.
wchar_t* zOptionsScreenHelpText[] = {
    // speech
    L"Włącz tę opcję, jeśli chcesz słuchać dialogów.",

    // Mute Confirmation
    L"Włącza lub wyłącza głosowe potwierzenia postaci.",

    // Subtitles
    L"Włącza lub wyłącza napisy podczas dialogów.",

    // Key to advance speech
    L"Jeśli napisy są włączone, opcja ta pozwoli ci spokojnie je przeczytać podczas dialogu.",

    // Toggle smoke animation
    L"Wyłącz tę opcję, aby poprawić płynność działania gry.",

    // Blood n Gore
    L"Wyłącz tę opcję, jeśli nie lubisz widoku krwi.",

    // Never move my mouse
    L"Wyłącz tę opcję, aby kursor myszki automatycznie ustawiał się nad pojawiającymi się "
    L"okienkami dialogowymi.",

    // Old selection method
    L"Włącz tę opcję, aby wybór postaci działał tak jak w poprzedniej wersji gry.",

    // Show movement path
    L"Włącz tę opcję jeśli chcesz widzieć trasę ruchu w trybie Real-Time.",

    // show misses
    L"Włącz tę opcję, aby zobaczyć w co trafiają twoje kule gdy pudłujesz.",

    // Real Time Confirmation
    L"Gdy opcja ta jest włączona, każdy ruch najemnika w trybie Real-Time będzie wymagał "
    L"dodatkowego, potwierdzającego kliknięcia.",

    // Sleep/Wake notification
    L"Gdy opcja ta jest włączona, wyświetlana będzie informacja, że najemnik położył się spać lub "
    L"wstał i wrócił do pracy.",

    // Use the metric system
    L"Gdy opcja ta jest włączona, gra używa systemu metrycznego.",

    // Merc Lighted movement
    L"Gdy opcja ta jest włączona, teren wokół najemnika będzie oświetlony podczas ruchu. Wyłącz tę "
    L"opcję, jeśli obniża płynność gry.",

    // Smart cursor
    L"Gdy opcja ta jest włączona, kursor automatycznie ustawia się na najemnikach gdy znajdzie się "
    L"w ich pobliżu.",

    // snap cursor to the door
    L"Gdy opcja ta jest włączona, kursor automatycznie ustawi się na drzwiach gdy znajdzie się w "
    L"ich pobliżu.",

    // glow items
    L"Gdy opcja ta jest włączona, przedmioty pulsują. ( |I )",

    // toggle tree tops
    L"Gdy opcja ta jest włączona, wyświetlane są korony drzew. ( |T )",

    // toggle wireframe
    L"Gdy opcja ta jest włączona, wyświetlane są zarysy niewidocznych ścian. ( |W )",

    L"Gdy opcja ta jest włączona, kursor ruchu wyświetlany jest w 3D. ( |Home )",

};

wchar_t* gzGIOScreenText[] = {
    L"POCZĄTKOWE USTAWIENIA GRY",
    L"Styl gry",
    L"Realistyczny",
    L"S-F",
    L"Opcje broni",
    L"Mnóstwo broni",
    L"Standardowe uzbrojenie",
    L"Stopień trudności",
    L"Nowicjusz",
    L"Doświadczony",
    L"Ekspert",
    L"Ok",
    L"Anuluj",
    L"Dodatkowe opcje",
    L"Nielimitowany czas",
    L"Tury limitowane czasowo",
    L"Nie działa w wersji demo",
};

wchar_t* pDeliveryLocationStrings[] = {
    L"Austin",       // Austin, Texas, USA
    L"Bagdad",       // Baghdad, Iraq (Suddam Hussein's home)
    L"Drassen",      // The main place in JA2 that you can receive items.  The other towns are dummy
                     // names...
    L"Hong Kong",    // Hong Kong, Hong Kong
    L"Bejrut",       // Beirut, Lebanon	(Middle East)
    L"Londyn",       // London, England
    L"Los Angeles",  // Los Angeles, California, USA (SW corner of USA)
    L"Meduna",       // Meduna -- the other airport in JA2 that you can receive items.
    L"Metavira",     // The island of Metavira was the fictional location used by JA1
    L"Miami",        // Miami, Florida, USA (SE corner of USA)
    L"Moskwa",       // Moscow, USSR
    L"Nowy Jork",    // New York, New York, USA
    L"Ottawa",       // Ottawa, Ontario, Canada -- where JA2 was made!
    L"Paryż",        // Paris, France
    L"Trypolis",     // Tripoli, Libya (eastern Mediterranean)
    L"Tokio",        // Tokyo, Japan
    L"Vancouver",    // Vancouver, British Columbia, Canada (west coast near US border)
};

wchar_t* pSkillAtZeroWarning[] = {
    // This string is used in the IMP character generation.  It is possible to select 0 ability
    // in a skill meaning you can't use it.  This text is confirmation to the player.
    L"Na pewno? Wartość zero oznacza brak jakichkolwiek umiejętności w tej dziedzinie.",
};

wchar_t* pIMPBeginScreenStrings[] = {
    L"( Maks. 8 znaków )",
};

wchar_t* pIMPFinishButtonText[1] = {
    L"Analizuję",
};

wchar_t* pIMPFinishStrings[] = {
    L"Dziękujemy, %s",  //%s is the name of the merc
};

// the strings for imp voices screen
wchar_t* pIMPVoicesStrings[] = {
    L"Głos",
};

wchar_t* pDepartedMercPortraitStrings[] = {
    L"Śmierć w akcji",
    L"Zwolnienie",
    L"Inny",
};

// title for program
wchar_t* pPersTitleText[] = {
    L"Personel",
};

// paused game strings
wchar_t* pPausedGameText[] = {
    L"Gra wstrzymana",
    L"Wznów grę (|P|a|u|s|e)",
    L"Wstrzymaj grę (|P|a|u|s|e)",
};

wchar_t* pMessageStrings[] = {
    L"Zakończyć grę?",
    L"OK",
    L"TAK",
    L"NIE",
    L"ANULUJ",
    L"NAJMIJ",
    L"LIE",
    L"Brak opisu",  // Save slots that don't have a description.
    L"Gra zapisana.",
    L"Gra zapisana.",
    L"Szybki zapis",    // The name of the quicksave file (filename, text reference)
    L"SaveGame",        // The name of the normal savegame file, such as SaveGame01,
                        // SaveGame02, etc.
    L"sav",             // The 3 character dos extension (represents sav)
    L"..\\SavedGames",  // The name of the directory where games are saved.
    L"Dzień",
    L"Najemn.",
    L"Wolna pozycja",  // An empty save game slot
    L"Demo",           // Demo of JA2
    L"Debug",          // State of development of a project (JA2) that is a debug build
    L"",               // Release build for JA2
    L"strz/min",       // Abbreviation for Rounds per minute -- the potential # of bullets
                       // fired in a minute.
    L"min",            // Abbreviation for minute.
    L"m",              // One character abbreviation for meter (metric distance measurement unit).
    L"kul",            // Abbreviation for rounds (# of bullets)
    L"kg",             // Abbreviation for kilogram (metric weight measurement unit)
    L"lb",             // Abbreviation for pounds (Imperial weight measurement unit)
    L"Strona główna",  // Home as in homepage on the internet.
    L"USD",            // Abbreviation to US dollars
    L"N/D",            // Lowercase acronym for not applicable.
    L"Tymczasem",      // Meanwhile
    L"%s przybył(a) do sektora %s%s",  // Name/Squad has arrived in sector A9.  Order
                                       // must not change without notifying SirTech
    L"Wersja",
    L"Wolna pozycja na szybki zapis",
    L"Ta pozycja zarezerwowana jest na szybkie zapisy wykonywane podczas gry "
    L"kombinacją klawiszy ALT+S.",
    L"Otw.",
    L"Zamkn.",
    L"Brak miejsca na dysku twardym.  Na dysku wolne jest %s MB, a wymagane jest "
    L"przynajmniej %s MB.",
    L"Najęto - %s z A.I.M.",
    L"%s złapał(a) %s",  //'Merc name' has caught 'item' -- let SirTech know if name
                         // comes after item.
    L"%s zaaplikował(a) sobie lekarstwo",  //'Merc name' has taken the drug
    L"%s nie posiada wiedzy medycznej",    //'Merc name' has no medical skill.

    // CDRom errors (such as ejecting CD while attempting to read the CD)
    L"Integralność gry została narażona na szwank.",
    L"BŁĄD: Wyjęto płytę CD",

    // When firing heavier weapons in close quarters, you may not have enough room to do
    // so.
    L"Nie ma miejsca, żeby stąd oddać strzał.",

    // Can't change stance due to objects in the way...
    L"Nie można zmienić pozycji w tej chwili.",

    // Simple text indications that appear in the game, when the merc can do one of
    // these things.
    L"Upuść",
    L"Rzuć",
    L"Podaj",

    L"%s przekazano do - %s.",  //"Item" passed to "merc".  Please try to keep the item
                                //%s before the merc %s, otherwise, must notify SirTech.
    L"Brak wolnego miejsca, by przekazać %s do - %s.",  // pass "item" to "merc".  Same
                                                        // instructions as above.

    // A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate
    // 'Attached )'
    L" dołączono )",

    // Cheat modes
    L"Pierwszy poziom lamerskich zagrywek osiągnięty",
    L"Drugi poziom lamerskich zagrywek osiągnięty",

    // Toggling various stealth modes
    L"Oddział ma włączony tryb skradania się.",
    L"Oddział ma wyłączony tryb skradania się.",
    L"%s ma włączony tryb skradania się.",
    L"%s ma wyłączony tryb skradania się.",

    // Wireframes are shown through buildings to reveal doors and windows that can't
    // otherwise be seen in an isometric engine.  You can toggle this mode freely in the
    // game.
    L"Dodatkowe siatki włączone.",
    L"Dodatkowe siatki wyłączone.",

    // These are used in the cheat modes for changing levels in the game.  Going from a
    // basement level to an upper level, etc.
    L"Nie można wyjść do góry z tego poziomu...",
    L"Nie ma już niższych poziomów...",
    L"Wejście na %d poziom pod ziemią...",
    L"Wyjście z podziemii...",

    L" - ",  // used in the shop keeper inteface to mark the ownership of the item eg
             // Red's gun
    L"Automatyczne centrowanie ekranu wyłączone.",
    L"Automatyczne centrowanie ekranu włączone.",
    L"Kursor 3D wyłączony.",
    L"Kursor 3D włączony.",
    L"Oddział %d aktywny.",
    L"%s - Nie stać cię by wypłacić jej/jemu dzienną pensję w wysokości %s.",  // first
                                                                               // %s is
                                                                               // the
                                                                               // mercs
                                                                               // name,
                                                                               // the
                                                                               // seconds
                                                                               // is a
                                                                               // string
                                                                               // containing
                                                                               // the
                                                                               // salary
    L"Pomiń",
    L"%s nie może odejść sam(a).",
    L"Utworzono zapis gry o nazwie SaveGame99.sav. W razie potrzeby zmień jego nazwę "
    L"na SaveGame01..10. Wtedy będzie można go odczytać ze standardowego okna odczytu "
    L"gry.",
    L"%s wypił(a) trochę - %s",
    L"Przesyłka dotarła do Drassen.",
    L"%s przybędzie do wyznaczonego punktu zrzutu (sektor %s) w dniu %d, około godziny "
    L"%s.",  // first %s is mercs name, next is the sector location and name where they
             // will be arriving in, lastely is the day an the time of arrival
    L"Lista historii zaktualizowana.",
#ifdef JA2BETAVERSION
    L"Automatyczny zapis został pomyślnie wykonany.",
#endif
};

uint16_t ItemPickupHelpPopup[][40] = {
    L"OK", L"W górę", L"Wybierz wszystko", L"W dół", L"Anuluj",
};

wchar_t* pDoctorWarningString[] = {
    L"%s jest za daleko, aby poddać się leczeniu.",
    L"Lekarze nie mogli opatrzyć wszystkich rannych.",
};

wchar_t* pMilitiaButtonsHelpText[] = {
    L"Podnieś(Prawy klawisz myszy)/upuść(Lewy klawisz myszy) Zielonych żołnierzy",  // button help
                                                                                    // text
                                                                                    // informing
                                                                                    // player they
                                                                                    // can pick up
                                                                                    // or drop
                                                                                    // militia with
                                                                                    // this button
    L"Podnieś(Prawy klawisz myszy)/upuść(Lewy klawisz myszy) Doświadczonych żołnierzy",
    L"Podnieś(Prawy klawisz myszy)/upuść(Lewy klawisz myszy) Weteranów",
    L"Umieszcza jednakową ilość żołnierzy samoobrony w każdym sektorze.",
};

wchar_t* pMapScreenJustStartedHelpText[] = {
    L"Zajrzyj do A.I.M. i zatrudnij kilku najemników (*Wskazówka* musisz otworzyć "
    L"laptopa)",  // to inform the player to hired some mercs to get things going
    L"Jeśli chcesz już udać się do Arulco, kliknij przycisk kompresji czasu, w prawym "
    L"dolnym rogu ekranu.",  // to inform the player to hit time compression to get the
                             // game underway
};

wchar_t* pAntiHackerString[] = {
    L"Błąd. Brakuje pliku, lub jest on uszkodzony. Gra zostanie przerwana.",
};

wchar_t* gzLaptopHelpText[] = {
    // Buttons:
    L"Przeglądanie poczty",
    L"Przeglądanie stron internetowych",
    L"Przeglądanie plików i załączników pocztowych",
    L"Rejestr zdarzeń",
    L"Informacje o członkach oddziału",
    L"Finanse i rejestr transakcji",
    L"Koniec pracy z laptopem",

    // Bottom task bar icons (if they exist):
    L"Masz nową pocztę",
    L"Masz nowe pliki",

    // Bookmarks:
    L"Międzynarodowe Stowarzyszenie Najemników",
    L"Bobby Ray's - Internetowy sklep z bronią",
    L"Instytut Badań Najemników",
    L"Bardziej Ekonomiczne Centrum Rekrutacyjne",
    L"McGillicutty's - Zakład pogrzebowy",
    L"United Floral Service",
    L"Brokerzy ubezpieczeniowi",
};

wchar_t* gzHelpScreenText[] = {
    L"Zamknij okno pomocy",
};

wchar_t* gzNonPersistantPBIText[] = {
    L"Trwa walka. Najemników można wycofać tylko na ekranie taktycznym.",
    L"W|ejdź do sektora, aby kontynuować walkę.",
    L"|Automatycznie rozstrzyga walkę.",
    L"Nie można automatycznie rozstrzygnąć walki, gdy atakujesz.",
    L"Nie można automatycznie rozstrzygnąć walki, gdy wpadasz w pułapkę.",
    L"Nie można automatycznie rozstrzygnąć walki, gdy walczysz ze stworzeniami w kopalni.",
    L"Nie można automatycznie rozstrzygnąć walki, gdy w sektorze są wrodzy cywile.",
    L"Nie można automatycznie rozstrzygnąć walki, gdy w sektorze są dzikie koty.",
    L"TRWA WALKA",
    L"W tym momencie nie możesz się wycofać.",
};

wchar_t* gzMiscString[] = {
    L"Żołnierze samoobrony kontynuują walkę bez pomocy twoich najemników...",
    L"W tym momencie tankowanie nie jest konieczne.",
    L"W baku jest %d%% paliwa.",
    L"Żołnierze Deidranny przejęli całkowitą kontrolę nad - %s.",
    L"Nie masz już gdzie zatankować.",
};

wchar_t* gzIntroScreen[] = {
    L"Nie odnaleziono filmu wprowadzającego",
};

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
wchar_t* pNewNoiseStr[] = {
    L"%s słyszy %s DŹWIĘK dochodzący z %s.",
    L"%s słyszy %s ODGŁOS RUCHU dochodzący z %s.",
    L"%s słyszy %s ODGŁOS SKRZYPNIĘCIA dochodzący z %s.",
    L"%s słyszy %s PLUSK dochodzący z %s.",
    L"%s słyszy %s ODGŁOS UDERZENIA dochodzący z %s.",
    L"%s słyszy %s WYBUCH dochodzący z %s.",
    L"%s słyszy %s KRZYK dochodzący z %s.",
    L"%s słyszy %s ODGŁOS UDERZENIA dochodzący z %s.",
    L"%s słyszy %s ODGŁOS UDERZENIA dochodzący z %s.",
    L"%s słyszy %s ŁOMOT dochodzący z %s.",
    L"%s słyszy %s TRZASK dochodzący z %s.",
};

wchar_t* wMapScreenSortButtonHelpText[] = {
    L"Sortuj według kolumny Imię (|F|1)",        L"Sortuj według kolumny Przydział (|F|2)",
    L"Sortuj według kolumny Sen (|F|3)",         L"Sortuj według kolumny Lokalizacja (|F|4)",
    L"Sortuj według kolumny Cel podróży (|F|5)", L"Sortuj według kolumny Wyjazd (|F|6)",
};

wchar_t* BrokenLinkText[] = {
    L"Błąd 404",
    L"Nie odnaleziono strony.",
};

wchar_t* gzBobbyRShipmentText[] = {
    L"Ostatnie dostawy",
    L"Zamówienie nr ",
    L"Ilość przedmiotów",
    L"Zamówiono:",
};

wchar_t* gzCreditNames[] = {
    L"Chris Camfield",
    L"Shaun Lyng",
    L"Kris Märnes",
    L"Ian Currie",
    L"Linda Currie",
    L"Eric \"WTF\" Cheng",
    L"Lynn Holowka",
    L"Norman \"NRG\" Olsen",
    L"George Brooks",
    L"Andrew Stacey",
    L"Scot Loving",
    L"Andrew \"Big Cheese\" Emmons",
    L"Dave \"The Feral\" French",
    L"Alex Meduna",
    L"Joey \"Joeker\" Whelan",
};

wchar_t* gzCreditNameTitle[] = {
    L"Game Internals Programmer",                    // Chris Camfield
    L"Co-designer/Writer",                           // Shaun Lyng
    L"Strategic Systems & Editor Programmer",        // Kris Marnes
    L"Producer/Co-designer",                         // Ian Currie
    L"Co-designer/Map Designer",                     // Linda Currie
    L"Artist",                                       // Eric \"WTF\" Cheng
    L"Beta Coordinator, Support",                    // Lynn Holowka
    L"Artist Extraordinaire",                        // Norman \"NRG\" Olsen
    L"Sound Guru",                                   // George Brooks
    L"Screen Designer/Artist",                       // Andrew Stacey
    L"Lead Artist/Animator",                         // Scot Loving
    L"Lead Programmer",                              // Andrew \"Big Cheese Doddle\" Emmons
    L"Programmer",                                   // Dave French
    L"Strategic Systems & Game Balance Programmer",  // Alex Meduna
    L"Portraits Artist",                             // Joey \"Joeker\" Whelan",
};

wchar_t* gzCreditNameFunny[] = {
    L"",                                       // Chris Camfield
    L"(still learning punctuation)",           // Shaun Lyng
    L"(\"It's done. I'm just fixing it\")",    // Kris \"The Cow Rape Man\" Marnes
    L"(getting much too old for this)",        // Ian Currie
    L"(and working on Wizardry 8)",            // Linda Currie
    L"(forced at gunpoint to also do QA)",     // Eric \"WTF\" Cheng
    L"(Left us for the CFSA - go figure...)",  // Lynn Holowka
    L"",                                       // Norman \"NRG\" Olsen
    L"",                                       // George Brooks
    L"(Dead Head and jazz lover)",             // Andrew Stacey
    L"(his real name is Robert)",              // Scot Loving
    L"(the only responsible person)",          // Andrew \"Big Cheese Doddle\" Emmons
    L"(can now get back to motocrossing)",     // Dave French
    L"(stolen from Wizardry 8)",               // Alex Meduna
    L"(did items and loading screens too!)",   // Joey \"Joeker\" Whelan",
};

wchar_t* sRepairsDoneString[] = {
    L"%s skończył(a) naprawiać własne wyposażenie",
    L"%s skończył(a) naprawiać broń i ochraniacze wszystkich członków oddziału",
    L"%s skończył(a) naprawiać wyposażenie wszystkich członków oddziału",
    L"%s skończył(a) naprawiać ekwipunek wszystkich członków oddziału",
};

wchar_t* zGioDifConfirmText[] = {
    L"Wybrano opcję Nowicjusz. Opcja ta jest przeznaczona dla niedoświadczonych graczy, lub dla "
    L"tych, którzy nie mają ochoty na długie i ciężkie walki. Pamiętaj, że opcja ta ma wpływ na "
    L"przebieg całej gry. Czy na pewno chcesz grać w trybie Nowicjusz?",
    L"Wybrano opcję Doświadczony. Opcja ta jest przenaczona dla graczy posiadających już pewne "
    L"doświadczenie w grach tego typu. Pamiętaj, że opcja ta ma wpływ na przebieg całej gry. Czy "
    L"na pewno chcesz grać w trybie Doświadczony?",
    L"Wybrano opcję Ekspert. Jakby co, to ostrzegaliśmy cię. Nie obwiniaj nas, jeśli wrócisz w "
    L"plastikowym worku. Pamiętaj, że opcja ta ma wpływ na przebieg całej gry. Czy na pewno chcesz "
    L"grać w trybie Ekspert?",
};

wchar_t* gzLateLocalizedString[] = {
    L"%S - nie odnaleziono pliku...",

    // 1-5
    L"Robot nie może opuścić sektora bez operatora.",

    // This message comes up if you have pending bombs waiting to explode in tactical.
    L"Nie można teraz kompresować czasu.  Poczekaj na fajerwerki!",

    //'Name' refuses to move.
    L"%s nie chce się przesunąć.",

    //%s a merc name
    L"%s ma zbyt mało energii, aby zmienić pozycję.",

    // A message that pops up when a vehicle runs out of gas.
    L"%s nie ma paliwa i stoi w sektorze %c%d.",

    // 6-10

    // the following two strings are combined with the pNewNoise[] strings above to report noises
    // heard above or below the merc
    L"GÓRY",
    L"DOŁU",

    // The following strings are used in autoresolve for autobandaging related feedback.
    L"Żaden z twoich najemników nie posiada wiedzy medycznej.",
    L"Brak środków medycznych, aby założyć rannym opatrunki.",
    L"Zabrakło środków medycznych, aby założyć wszystkim rannym opatrunki.",
    L"Żaden z twoich najemników nie potrzebuje pomocy medycznej.",
    L"Automatyczne zakładanie opatrunków rannym najemnikom.",
    L"Wszystkim twoim najemnikom założono opatrunki.",

    // 14
    L"Arulco",

    L"(dach)",

    L"Zdrowie: %d/%d",

    // In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
    //"vs." is the abbreviation of versus.
    L"%d vs. %d",

    L"%s - brak wolnych miejsc!",  //(ex "The ice cream truck is full")

    L"%s nie potrzebuje pierwszej pomocy lecz opieki lekarza lub dłuższego odpoczynku.",

    // 20
    // Happens when you get shot in the legs, and you fall down.
    L"%s dostał(a) w nogi i upadł(a)!",
    // Name can't speak right now.
    L"%s nie może teraz mówić.",

    // 22-24 plural versions
    L"%d zielonych żołnierzy samoobrony awansowało na weteranów.",
    L"%d zielonych żołnierzy samoobrony awansowało na regularnych żołnierzy.",
    L"%d regularnych żołnierzy samoobrony awansowało na weteranów.",

    // 25
    L"Przełącznik",

    // 26
    // Name has gone psycho -- when the game forces the player into burstmode (certain unstable
    // characters)
    L"%s dostaje świra!",

    // 27-28
    // Messages why a player can't time compress.
    L"Niebezpiecznie jest kompresować teraz czas, gdyż masz najemników w sektorze %s.",
    L"Niebezpiecznie jest kompresować teraz czas, gdyż masz najemników w kopalni zaatakowanej "
    L"przez robale.",

    // 29-31 singular versions
    L"1 zielony żołnierz samoobrony awansował na weterana.",
    L"1 zielony żołnierz samoobrony awansował na regularnego żołnierza.",
    L"1 regularny żołnierz samoobrony awansował na weterana.",

    // 32-34
    L"%s nic nie mówi.",
    L"Wyjść na powierzchnię?",
    L"(Oddział %d)",

    // 35
    // Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope,
    // Scope before MP5K)
    L"%s naprawił(a) najemnikowi - %s, jego/jej - %s",

    // 36
    L"DZIKI KOT",

    // 37-38 "Name trips and falls"
    L"%s potyka się i upada",
    L"Nie można stąd podnieść tego przedmiotu.",

    // 39
    L"Żaden z twoich najemników nie jest w stanie walczyć.  Żołnierze samoobrony sami będą walczyć "
    L"z robalami.",

    // 40-43
    //%s is the name of merc.
    L"%s nie ma środków medycznych!",
    L"%s nie posiada odpowiedniej wiedzy, aby kogokolwiek wyleczyć!",
    L"%s nie ma narzędzi!",
    L"%s nie posiada odpowiedniej wiedzy, aby cokolwiek naprawić!",

    // 44-45
    L"Czas naprawy",
    L"%s nie widzi tej osoby.",

    // 46-48
    L"%s - przedłużka lufy jego/jej broni odpada!",
    L"W jednym sektorze, szkolenie samoobrony może prowadzić tylko %d instruktor(ów).",
    L"Na pewno?",

    // 49-50
    L"Kompresja czasu",
    L"Pojazd ma pełny zbiornik paliwa.",

    // 51-52 Fast help text in mapscreen.
    L"Kontynuuj kompresję czasu (|S|p|a|c|j|a)",
    L"Zatrzymaj kompresję czasu (|E|s|c)",

    // 53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
    L"%s odblokował(a) - %s",
    L"%s odblokował(a) najemnikowi - %s, jego/jej - %s",

    // 55
    L"Nie można kompresować czasu, gdy otwarty jest inwentarz sektora.",

    L"Nie odnaleziono płyty nr 2 Jagged Alliance 2.",

    L"Przedmioty zostały pomyślnie połączone.",

    // 58
    // Displayed with the version information when cheats are enabled.
    L"Bieżący/Maks. postęp: %d%%/%d%%",

    // 59
    L"Eskortować Johna i Mary?",

    L"Przełącznik aktywowany.",
};

#endif  // POLISH
