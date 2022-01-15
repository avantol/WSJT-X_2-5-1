#include "about.h"

#include <QCoreApplication>
#include <QString>

#include "revision_utils.hpp"

#include "ui_about.h"

CAboutDlg::CAboutDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::CAboutDlg)
{
  ui->setupUi(this);

  ui->labelTxt->setText ("<h2>" + QString {"WSJT-X v"
                                             + QCoreApplication::applicationVersion ()
                                             + " " + revision ()}.simplified () + " (<a href=\"https://github.com/avantol/WSJT-X_2-5-1\">modified by WM8Q</a>)</h2><br />"
    "WSJT-X implements a number of digital modes designed for <br />"
    "weak-signal Amateur Radio communication.  <br /><br />"
    "&copy; 2001-2021 by Joe Taylor, K1JT, Bill Somerville, G4WJS, <br />"
    "Steve Franke, K9AN, and Nico Palermo, IV3NWV.  <br /><br />"
    "We gratefully acknowledge contributions from AC6SL, AE4JY,<br />"
    "DF2ET, DJ0OT, G3WDG, G4KLA, IW3RAB, K3WYC, KA1GT,<br />"
    "KA6MAL, KA9Q, KB1ZMX, KD6EKQ, KI7MT, KK1D, ND0B, PY2SDR,<br />"
    "VE1SKY, VK3ACF, VK4BDJ, VK7MO, W3DJS, W3SZ, W4TI, W4TV,<br />"
    "and W9MDB.<br /><br />"
    "WSJT-X is licensed under the terms of Version 3 <br />"
    "of the GNU General Public License (GPL) <br /><br />"
    "<a href=" TO_STRING__ (PROJECT_HOMEPAGE) ">"
    "<img src=\":/icon_128x128.png\" /></a>"
    "<a href=\"https://www.gnu.org/licenses/gpl-3.0.txt\">"
    "<img src=\":/gpl-v3-logo.svg\" height=\"80\" /><br />"
    "https://www.gnu.org/licenses/gpl-3.0.txt</a>");
}

CAboutDlg::~CAboutDlg()
{
}
