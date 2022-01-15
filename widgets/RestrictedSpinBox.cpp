#include "RestrictedSpinBox.hpp"

#include <algorithm>
#include <utility>

#include <QString>

QValidator::State RestrictedSpinBox::validate (QString& input, int& pos) const
{
  // start by doing the standard QSpinBox validation
  auto valid = HintedSpinBox::validate (input, pos);
  // extra validation
  if (QValidator::Acceptable == valid
      && values ().end () == std::find (values ().begin (), values ().end (), valueFromText (input)))
    {
      valid = QValidator::Intermediate;
    }
  return valid;
}

void RestrictedSpinBox::fixup (QString& input) const
{
  auto iter = std::lower_bound (values ().begin (), values ().end (), valueFromText (input));
  HintedSpinBox::fixup (input);
  if (iter != values ().end ())
    {
      input = textFromValue (*iter);
    }
  else
    {
      input = textFromValue (values ().back ());
    }
}
