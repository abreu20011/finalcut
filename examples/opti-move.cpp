/***********************************************************************
* opti-move.cpp - Tests the cursor movement optimization               *
*                                                                      *
* This file is part of the Final Cut widget toolkit                    *
*                                                                      *
* Copyright 2016-2020 Markus Gans                                      *
*                                                                      *
* The Final Cut is free software; you can redistribute it and/or       *
* modify it under the terms of the GNU Lesser General Public License   *
* as published by the Free Software Foundation; either version 3 of    *
* the License, or (at your option) any later version.                  *
*                                                                      *
* The Final Cut is distributed in the hope that it will be useful,     *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU Lesser General Public License for more details.                  *
*                                                                      *
* You should have received a copy of the GNU Lesser General Public     *
* License along with this program.  If not, see                        *
* <http://www.gnu.org/licenses/>.                                      *
***********************************************************************/

#include <iomanip>
#include <string>

#include <final/final.h>


// Global FApplication object
static finalcut::FApplication* app{nullptr};

// function prototype
bool keyPressed();
void term_boundaries (int&, int&);
void move (int, int, int, int);


//----------------------------------------------------------------------
// functions
//----------------------------------------------------------------------
bool keyPressed()
{
  // Waiting for keypress

  struct termios save{};
  bool ret{false};
  std::cout << "\nPress any key to continue...";
  fflush(stdout);
  tcgetattr (STDIN_FILENO, &save);
  struct termios t = save;
  t.c_lflag &= uInt(~(ICANON | ECHO));
  tcsetattr (STDIN_FILENO, TCSANOW, &t);

  if ( std::getchar() != EOF )
    ret = true;
  else
    ret = false;

  tcsetattr (STDIN_FILENO, TCSADRAIN, &save);
  return ret;
}

//----------------------------------------------------------------------
void term_boundaries (int& x, int& y)
{
  // checks and corrects the terminal boundaries
  const int term_width  = int(app->getDesktopWidth());
  const int term_height = int(app->getDesktopHeight());

  if ( x < 0 )
    x = 0;

  if ( y < 0 )
    y = 0;

  if ( x >= term_width && term_width > 0 )
  {
    y += x / term_width;
    x %= term_width;
  }

  if ( y >= term_height )
    y = term_height - 1;
}

//----------------------------------------------------------------------
void move (int xold, int yold, int xnew, int ynew)
{
  // Prints the cursor move escape sequence
  finalcut::FString buffer{};
  finalcut::FString sequence{};
  finalcut::FString from{};
  finalcut::FString to{};
  finalcut::FString byte{};
  const std::string ctrl_character[] =
  {
    "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
    "BS",  "Tab", "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
    "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
    "CAN", "EM",  "SUB", "Esc", "FS",  "GS",  "RS",  "US",
    "Space"
  };

  term_boundaries(xold, yold);
  term_boundaries(xnew, ynew);

  // Get the move string
  buffer = finalcut::FTerm::moveCursorString (xold, yold, xnew, ynew);

  for (auto&& ch : buffer)
  {
    if ( ch < 0x21 )
      sequence += ctrl_character[std::size_t(ch)];
    else
      sequence += ch;

    sequence += ' ';
  }

  from.sprintf ("(%3d;%3d)", xold, yold);
  to.sprintf ("(%3d;%3d)", xnew, ynew);
  const std::size_t len = buffer.getLength();

  if ( len <= 1 )
    byte.sprintf ("%d byte ", len);
  else
    byte.sprintf ("%d bytes", len);

  std::cout << std::right << std::setw(10) << from << " -> "
            << std::left << std::setw(10) << to << " "
            << std::setw(21) << sequence << " "
            << std::right << std::setw(10) << byte << "\r\n";
}


//----------------------------------------------------------------------
//                               main part
//----------------------------------------------------------------------
int main (int argc, char* argv[])
{
  // Create the application object
  finalcut::FApplication TermApp{argc, argv};

  // Pointer to the global virtual terminal object
  app = &TermApp;

  // Get screen dimension
  int xmax = int(TermApp.getDesktopWidth() - 1);
  int ymax = int(TermApp.getDesktopHeight() - 1);
  finalcut::FString line{std::size_t(xmax) + 1, '-'};

  // Place the cursor in the upper left corner
  TermApp.setTermXY(0, 0);
  // Reset all terminal attributes
  TermApp.setNormal();
  // Clear the screen
  TermApp.clearArea();

  // Show the determined terminal name and text resolution
  std::cout << "Terminal: " << TermApp.getTermType() << "\r\n";
  std::cout << " Columns: 0.." << xmax << "\r\n";
  std::cout << "   Lines: 0.." << ymax << "\r\n";

  // Show the escape sequences for the following cursor movements
  std::cout << std::setw(38) << "Cursor move\r\n";
  std::cout << "    (From) -> (To)       ";
  std::cout << "escape sequence          ";
  std::cout << "Length\r\n";
  std::cout << line;

  move (5, 12, 0, 0);
  move (5, ymax, 5, 0);
  move (xmax, 1, 0, 1);
  move (xmax, 1, 0, 2);
  move (xmax + 1, 1, 0, 2);
  move (9, 4, 10, 4);
  move (10, 4, 9, 4);
  move (9, 4, 11, 4);
  move (11, 4, 9, 4);
  move (1, 0, 8, 0);
  move (16, 0, 16, 1);
  move (16, 1, 16, 0);
  move (16, 0, 16, 2);
  move (16, 2, 16, 0);
  move (3, 2, xmax, 2);
  move (5, 5, xmax - 5, ymax - 5);

  // Waiting for keypress
  keyPressed();

  // Show terminal speed and milliseconds for all cursor movement sequence
  std::cout << "\r" << line;
  const finalcut::FOptiMove& opti_move = *finalcut::FTerm::getFOptiMove();
  finalcut::printDurations(opti_move);

  // Waiting for keypress
  keyPressed();
  app = nullptr;  // End of TermApp object scope
}
