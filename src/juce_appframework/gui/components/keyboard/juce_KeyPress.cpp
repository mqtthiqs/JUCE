/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_KeyPress.h"
#include "../juce_Component.h"


//==============================================================================
KeyPress::KeyPress() throw()
    : keyCode (0),
      mods (0),
      textCharacter (0)
{
}

KeyPress::KeyPress (const int keyCode_,
                    const ModifierKeys& mods_,
                    const juce_wchar textCharacter_) throw()
    : keyCode (keyCode_),
      mods (mods_),
      textCharacter (textCharacter_)
{
}

KeyPress::KeyPress (const int keyCode_) throw()
    : keyCode (keyCode_),
      textCharacter (0)
{
}

KeyPress::KeyPress (const KeyPress& other) throw()
    : keyCode (other.keyCode),
      mods (other.mods),
      textCharacter (other.textCharacter)
{
}

const KeyPress& KeyPress::operator= (const KeyPress& other) throw()
{
    keyCode = other.keyCode;
    mods = other.mods;
    textCharacter = other.textCharacter;

    return *this;
}

bool KeyPress::operator== (const KeyPress& other) const throw()
{
    return mods.getRawFlags() == other.mods.getRawFlags()
            && (textCharacter == other.textCharacter
                 || textCharacter == 0
                 || other.textCharacter == 0)
            && (keyCode == other.keyCode
                 || (keyCode < 256
                      && other.keyCode < 256
                      && CharacterFunctions::toLowerCase ((tchar) keyCode)
                           == CharacterFunctions::toLowerCase ((tchar) other.keyCode)));
}

bool KeyPress::operator!= (const KeyPress& other) const throw()
{
    return ! operator== (other);
}

bool KeyPress::isCurrentlyDown() const
{
    int modsMask = ModifierKeys::commandModifier | ModifierKeys::ctrlModifier | ModifierKeys::altModifier;

    if (keyCode == KeyPress::downKey
        || keyCode == KeyPress::upKey
        || keyCode == KeyPress::leftKey
        || keyCode == KeyPress::rightKey
        || keyCode == KeyPress::deleteKey
        || keyCode == KeyPress::backspaceKey
        || keyCode == KeyPress::returnKey
        || keyCode == KeyPress::escapeKey
        || keyCode == KeyPress::homeKey
        || keyCode == KeyPress::endKey
        || keyCode == KeyPress::pageUpKey
        || keyCode == KeyPress::pageDownKey
        || (keyCode >= KeyPress::F1Key && keyCode <= KeyPress::F16Key))
    {
        modsMask |= ModifierKeys::shiftModifier;
    }

    return isKeyCurrentlyDown (keyCode)
            && (ModifierKeys::getCurrentModifiers().getRawFlags() & modsMask)
                  == (mods.getRawFlags() & modsMask);
}

//==============================================================================
struct KeyNameAndCode
{
    const char* name;
    int code;
};

static const KeyNameAndCode keyNameTranslations[] =
{
    { "spacebar",       KeyPress::spaceKey },
    { "return",         KeyPress::returnKey },
    { "escape",         KeyPress::escapeKey },
    { "backspace",      KeyPress::backspaceKey },
    { "cursor left",    KeyPress::leftKey },
    { "cursor right",   KeyPress::rightKey },
    { "cursor up",      KeyPress::upKey },
    { "cursor down",    KeyPress::downKey },
    { "page up",        KeyPress::pageUpKey },
    { "page down",      KeyPress::pageDownKey },
    { "home",           KeyPress::homeKey },
    { "end",            KeyPress::endKey },
    { "delete",         KeyPress::deleteKey },
    { "insert",         KeyPress::insertKey },
    { "tab",            KeyPress::tabKey },
    { "play",           KeyPress::playKey },
    { "stop",           KeyPress::stopKey },
    { "fast forward",   KeyPress::fastForwardKey },
    { "rewind",         KeyPress::rewindKey }
};

static const tchar* const numberPadPrefix = T("numpad ");

//==============================================================================
const KeyPress KeyPress::createFromDescription (const String& desc)
{
    int modifiers = 0;

    if (desc.containsWholeWordIgnoreCase (T("ctrl"))
         || desc.containsWholeWordIgnoreCase (T("control"))
         || desc.containsWholeWordIgnoreCase (T("ctl")))
        modifiers |= ModifierKeys::ctrlModifier;

    if (desc.containsWholeWordIgnoreCase (T("shift"))
         || desc.containsWholeWordIgnoreCase (T("shft")))
        modifiers |= ModifierKeys::shiftModifier;

    if (desc.containsWholeWordIgnoreCase (T("alt"))
         || desc.containsWholeWordIgnoreCase (T("option")))
        modifiers |= ModifierKeys::altModifier;

    if (desc.containsWholeWordIgnoreCase (T("command"))
         || desc.containsWholeWordIgnoreCase (T("cmd")))
        modifiers |= ModifierKeys::commandModifier;

    int key = 0;

    for (int i = 0; i < numElementsInArray (keyNameTranslations); ++i)
    {
        if (desc.containsWholeWordIgnoreCase (String (keyNameTranslations[i].name)))
        {
            key = keyNameTranslations[i].code;
            break;
        }
    }

    if (key == 0)
    {
        // see if it's a numpad key..
        for (int i = 0; i < 10; ++i)
            if (desc.containsWholeWordIgnoreCase (numberPadPrefix + String (i)))
                key = numberPad0 + i;

        if (key == 0)
        {
            // see if it's a function key..
            for (int i = 1; i <= 12; ++i)
                if (desc.containsWholeWordIgnoreCase (T("f") + String (i)))
                    key = F1Key + i - 1;

            if (key == 0)
            {
                // give up and use the hex code..
                const int hexCode = desc.fromFirstOccurrenceOf (T("#"), false, false)
                                        .toLowerCase()
                                        .retainCharacters (T("0123456789abcdef"))
                                        .getHexValue32();

                if (hexCode > 0)
                {
                    key = hexCode;
                }
                else
                {
                    key = CharacterFunctions::toUpperCase (desc.getLastCharacter());
                }
            }
        }
    }

    return KeyPress (key, ModifierKeys (modifiers), 0);
}

const String KeyPress::getTextDescription() const
{
    String desc;

    if (keyCode > 0)
    {
        if (mods.isCtrlDown())
            desc << "ctrl + ";

        if (mods.isShiftDown())
            desc << "shift + ";

#ifdef JUCE_MAC
          // only do this on the mac, because on Windows ctrl and command are the same,
          // and this would get confusing
          if (mods.isCommandDown())
              desc << "command + ";

        if (mods.isAltDown())
            desc << "option + ";
#else
        if (mods.isAltDown())
            desc << "alt + ";
#endif

        for (int i = 0; i < numElementsInArray (keyNameTranslations); ++i)
            if (keyCode == keyNameTranslations[i].code)
                return desc + keyNameTranslations[i].name;

        if (keyCode >= F1Key && keyCode <= F16Key)
            desc << 'F' << (1 + keyCode - F1Key);
        else if (keyCode >= numberPad0 && keyCode <= numberPad9)
            desc << numberPadPrefix << (keyCode - numberPad0);
        else if (keyCode >= 33 && keyCode < 176)
            desc += CharacterFunctions::toUpperCase ((tchar) keyCode);
        else
            desc << '#' << String::toHexString (keyCode);
    }

    return desc;
}

END_JUCE_NAMESPACE
