/* Copyright  (C) 2010-2019 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (file_path.h).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <playlists/label_sanitization.h>
#include <compat/strl.h>
#include <retro_miscellaneous.h>
#include <string/stdstring.h>
#include <string.h>

const size_t disc_strings_length = 3;

const char *disc_strings[3] = {
   "(CD",
   "(Disc",
   "(Disk"
};

const size_t region_strings_length = 20;

/*
 * We'll use the standard No-Intro regions for now.
 */
const char *region_strings[20] = {
   "(Australia)", /* Don’t use with Europe */
   "(Brazil)",
   "(Canada)",    /* Don’t use with USA */
   "(China)",
   "(France)",
   "(Germany)",
   "(Hong Kong)",
   "(Italy)",
   "(Japan)",
   "(Korea)",
   "(Netherlands)",
   "(Spain)",
   "(Sweden)",
   "(USA)",       /* Includes Canada */
   "(World)",
   "(Europe)",    /* Includes Australia */
   "(Asia)",
   "(Japan, USA)",
   "(Japan, Europe)",
   "(USA, Europe)"
};

/*
 * Does not work with nested blocks.
 */
void label_sanitize(char *label, bool (*left)(char*), bool (*right)(char*))
{
   bool copy = true;
   int rindex = 0;
   int lindex = 0;
   char new_label[PATH_MAX_LENGTH];

   for (; lindex < PATH_MAX_LENGTH && label[lindex] != '\0'; lindex++)
   {
      if (copy)
      {
         /* check for the start of the range */
         if ((*left)(&label[lindex]))
            copy = false;

         if (copy)
            new_label[rindex++] = label[lindex];
      }
      else if ((*right)(&label[lindex]))
         copy = true;
   }

   new_label[rindex] = '\0';

   strlcpy(label, new_label, PATH_MAX_LENGTH);
}

bool left_parens(char *left)
{
   return left[0] == '(';
}

bool right_parens(char *right)
{
   return right[0] == ')';
}

bool left_brackets(char *left)
{
   return left[0] == '[';
}

bool right_brackets(char *right)
{
   return right[0] == ']';
}

bool left_parens_or_brackets(char *left)
{
   return left[0] == '(' || left[0] == '[';
}

bool right_parens_or_brackets(char *right)
{
   return right[0] == ')' || right[0] == ']';
}

bool left_exclusion(char *left,
      const char **strings, const size_t strings_count)
{
   unsigned i;
   char exclusion_string[32];
   char comparison_string[32];

   strlcpy(exclusion_string, left, 32);
   string_to_upper(exclusion_string);

   for (i = 0; i < (unsigned)strings_count; i++)
   {
      strlcpy(comparison_string, strings[i], 32);
      string_to_upper(comparison_string);

      if (string_is_equal_fast(exclusion_string,
               comparison_string, strlen(comparison_string)))
         return true;
   }

   return false;
}

bool left_parens_or_brackets_excluding_region(char *left)
{
   return left_parens_or_brackets(left)
      && !left_exclusion(left, region_strings, region_strings_length);
}

bool left_parens_or_brackets_excluding_disc(char *left)
{
   return left_parens_or_brackets(left)
      && !left_exclusion(left, disc_strings, disc_strings_length);
}

bool left_parens_or_brackets_excluding_region_or_disc(char *left)
{
   return left_parens_or_brackets(left)
      && !left_exclusion(left, region_strings, region_strings_length)
      && !left_exclusion(left, disc_strings, disc_strings_length);
}

void label_remove_parens(char *label)
{
   label_sanitize(label, left_parens, right_parens);
}

void label_remove_brackets(char *label)
{
   label_sanitize(label, left_brackets, right_brackets);
}

void label_remove_parens_and_brackets(char *label)
{
   label_sanitize(label, left_parens_or_brackets, right_parens_or_brackets);
}

void label_keep_region(char *label)
{
   label_sanitize(label, left_parens_or_brackets_excluding_region, right_parens_or_brackets);
}

void label_keep_disc(char *label)
{
   label_sanitize(label, left_parens_or_brackets_excluding_disc, right_parens_or_brackets);
}

void label_keep_region_and_disc(char *label)
{
   label_sanitize(label, left_parens_or_brackets_excluding_region_or_disc, right_parens_or_brackets);
}
