================================================================================
    MFC 라이브러리 : EzSpice 프로젝트 개요
===============================================================================

응용 프로그램 마법사에서 이 EzSpice 응용 프로그램을 
만들었습니다.  이 응용 프로그램은 MFC(Microsoft Foundation Classes)의
기본적인 사용법을 보여 주며 응용 프로그램 작성을 위한 출발점을 제공합니다.

이 파일에는 EzSpice 응용 프로그램을 구성하는 각 파일에 대한
요약 설명이 포함되어 있습니다.

EzSpice.vcproj
    응용 프로그램 마법사를 사용하여 생성된 VC++ 프로젝트의 주 프로젝트 파일입니다.
    파일을 생성한 Visual C++ 버전에 대한 정보와
    응용 프로그램 마법사를 사용하여 선택한 플랫폼, 구성 및 프로젝트
    기능에 대한 정보가 들어 있습니다.

EzSpice.h
    응용 프로그램의 기본 헤더 파일입니다.  여기에는 다른 프로젝트 관련
    헤더(Resource.h 포함)가 들어 있고 CEzSpiceApp 응용 프로그램
    클래스를 선언합니다.

EzSpice.cpp
    응용 프로그램 클래스 CEzSpiceApp이(가) 들어 있는 기본 응용 프로그램
    소스 파일입니다.

EzSpice.rc
    프로그램에서 사용하는 모든 Microsoft Windows 리소스의 목록입니다.
  여기에는 RES 하위 디렉터리에 저장된 아이콘, 비트맵 및 커서가
    포함됩니다.  이 파일은 Microsoft Visual C++에서
    직접 편집할 수 있습니다. 프로젝트 리소스는 1042에 있습니다.

res\EzSpice.ico
    응용 프로그램의 아이콘으로 사용되는 아이콘 파일입니다.  이 아이콘은
    주 리소스 파일인 EzSpice.rc에 의해 포함됩니다.

res\EzSpice.rc2
    이 파일에는 Microsoft Visual C++ 이외의 다른 도구에서 편집한 리소스가
    포함되어 있습니다. 리소스 편집기로 편집할 수 없는 모든 리소스는
    이 파일에 넣어야 합니다.

/////////////////////////////////////////////////////////////////////////////

주 프레임 창의 경우:
    프로젝트에는 표준 MFC 인터페이스가 포함됩니다.

MainFrm.h, MainFrm.cpp
    이 파일에는 프레임 클래스 CMainFrame이(가) 들어 있습니다.
    이 클래스는 CMDIFrameWnd에서 파생되며 모든 MDI 프레임 기능을 제어합니다.

res\Toolbar.bmp
    이 비트맵 파일은 도구 모음을 위한 바둑판식 이미지를 만드는 데 사용됩니다.
    초기 도구 모음과 상태 표시줄은 CMainFrame
    클래스에서 생성됩니다. 리소스 편집기를 사용하여 이 도구 모음 비트맵을 편집하고
    EzSpice.rc의 IDR_MAINFRAME TOOLBAR 배열을 업데이트하여 도구 모음
    단추를 추가합니다.
/////////////////////////////////////////////////////////////////////////////

자식 프레임 창의 경우:

ChildFrm.h, ChildFrm.cpp
    이 파일은 MDI 응용 프로그램에서 자식 창을 지원하는 CChildFrame
    클래스를 정의하고 구현합니다.

/////////////////////////////////////////////////////////////////////////////

응용 프로그램 마법사에서 하나의 문서 형식과 하나의 뷰가 만들어집니다.

EzSpiceDoc.h, EzSpiceDoc.cpp - 문서
    이 파일에는 CEzSpiceDoc 클래스가 들어 있습니다.  이 파일을 편집하여
    특수한 문서 데이터를 추가하고 CEzSpiceDoc::Serialize를 통해 파일
    저장 및 로드를 구현합니다.

EzSpiceView.h, EzSpiceView.cpp - 문서 뷰
    이 파일에는 CEzSpiceView 클래스가 들어 있습니다.
    CEzSpiceView 개체는 CEzSpiceDoc 개체를 보는 데 사용됩니다.

res\EzSpiceDoc.ico
    CEzSpiceDoc 클래스의 MDI 자식 창에 대한 아이콘으로 사용되는
    아이콘 파일입니다.  이 아이콘은 주 리소스 파일인 EzSpice.rc에
    의해 포함됩니다.




/////////////////////////////////////////////////////////////////////////////

기타 기능:

ActiveX 컨트롤
    응용 프로그램에서 ActiveX 컨트롤을 사용할 수 있도록 지원합니다.

인쇄 및 인쇄 미리 보기 지원
    응용 프로그램 마법사가 MFC 라이브러리에서 CView 클래스의 멤버 함수를 호출하여 인쇄, 인쇄 설정
    및 인쇄 미리 보기 명령을 처리하는 코드를 생성했습니다.

/////////////////////////////////////////////////////////////////////////////

기타 표준 파일:

StdAfx.h, StdAfx.cpp
    이 파일은 미리 컴파일된 헤더(PCH) 파일인 EzSpice.pch와
    미리 컴파일된 형식 파일인 StdAfx.obj를 빌드하는 데 사용됩니다.

Resource.h
    새 리소스 ID를 정의하는 표준 헤더 파일입니다.
    Microsoft Visual C++에서 이 파일을 읽고 업데이트합니다.

EzSpice.manifest
	응용 프로그램 매니페스트 파일은 Windows XP에서 특정 버전의 Side-by-Side
	어셈블리에 대한 응용 프로그램 종속성을 설명하는 데 사용됩니다. 로더는 이 정보를
	사용하여 어셈블리 캐시에서 적절한 어셈블리를 로드하거나 응용 프로그램에서 전용
	어셈블리를 로드합니다. 응용 프로그램 매니페스트는 응용 프로그램 실행 파일과 같은
	폴더에 설치된 외부 .manifest 파일로 재배포를 위해 포함되거나 리소스의 형태로
	실행 파일에 포함될 수 있습니다.
/////////////////////////////////////////////////////////////////////////////

기타 참고:

응용 프로그램 마법사는 "TODO:"를 사용하여 추가하거나 사용자 지정해야 하는
소스 코드 부분을 나타냅니다.

응용 프로그램에서 공유 DLL의 MFC를 사용하는 경우
해당 MFC DLL을 다시 배포해야 합니다. 응용 프로그램의 언어가
운영 체제의 로캘과 다른 경우
지역화된 해당 리소스 MFC90XXX.DLL도 다시 배포해야 합니다.
이 두 가지 항목에 대한 자세한 내용은 MSDN 설명서에서
Visual C++ 응용 프로그램 다시 배포에 대한 단원을 참조하십시오.

/////////////////////////////////////////////////////////////////////////////


                    GNU GENERAL PUBLIC LICENSE
                       Version 2, June 1991

 Copyright (C) 1989, 1991 Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.

                            Preamble

  The licenses for most software are designed to take away your
freedom to share and change it.  By contrast, the GNU General Public
License is intended to guarantee your freedom to share and change free
software--to make sure the software is free for all its users.  This
General Public License applies to most of the Free Software
Foundation's software and to any other program whose authors commit to
using it.  (Some other Free Software Foundation software is covered by
the GNU Lesser General Public License instead.)  You can apply it to
your programs, too.

  When we speak of free software, we are referring to freedom, not
price.  Our General Public Licenses are designed to make sure that you
have the freedom to distribute copies of free software (and charge for
this service if you wish), that you receive source code or can get it
if you want it, that you can change the software or use pieces of it
in new free programs; and that you know you can do these things.

  To protect your rights, we need to make restrictions that forbid
anyone to deny you these rights or to ask you to surrender the rights.
These restrictions translate to certain responsibilities for you if you
distribute copies of the software, or if you modify it.

  For example, if you distribute copies of such a program, whether
gratis or for a fee, you must give the recipients all the rights that
you have.  You must make sure that they, too, receive or can get the
source code.  And you must show them these terms so they know their
rights.

  We protect your rights with two steps: (1) copyright the software, and
(2) offer you this license which gives you legal permission to copy,
distribute and/or modify the software.

  Also, for each author's protection and ours, we want to make certain
that everyone understands that there is no warranty for this free
software.  If the software is modified by someone else and passed on, we
want its recipients to know that what they have is not the original, so
that any problems introduced by others will not reflect on the original
authors' reputations.

  Finally, any free program is threatened constantly by software
patents.  We wish to avoid the danger that redistributors of a free
program will individually obtain patent licenses, in effect making the
program proprietary.  To prevent this, we have made it clear that any
patent must be licensed for everyone's free use or not licensed at all.

  The precise terms and conditions for copying, distribution and
modification follow.

                    GNU GENERAL PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. This License applies to any program or other work which contains
a notice placed by the copyright holder saying it may be distributed
under the terms of this General Public License.  The "Program", below,
refers to any such program or work, and a "work based on the Program"
means either the Program or any derivative work under copyright law:
that is to say, a work containing the Program or a portion of it,
either verbatim or with modifications and/or translated into another
language.  (Hereinafter, translation is included without limitation in
the term "modification".)  Each licensee is addressed as "you".

Activities other than copying, distribution and modification are not
covered by this License; they are outside its scope.  The act of
running the Program is not restricted, and the output from the Program
is covered only if its contents constitute a work based on the
Program (independent of having been made by running the Program).
Whether that is true depends on what the Program does.

  1. You may copy and distribute verbatim copies of the Program's
source code as you receive it, in any medium, provided that you
conspicuously and appropriately publish on each copy an appropriate
copyright notice and disclaimer of warranty; keep intact all the
notices that refer to this License and to the absence of any warranty;
and give any other recipients of the Program a copy of this License
along with the Program.

You may charge a fee for the physical act of transferring a copy, and
you may at your option offer warranty protection in exchange for a fee.

  2. You may modify your copy or copies of the Program or any portion
of it, thus forming a work based on the Program, and copy and
distribute such modifications or work under the terms of Section 1
above, provided that you also meet all of these conditions:

    a) You must cause the modified files to carry prominent notices
    stating that you changed the files and the date of any change.

    b) You must cause any work that you distribute or publish, that in
    whole or in part contains or is derived from the Program or any
    part thereof, to be licensed as a whole at no charge to all third
    parties under the terms of this License.

    c) If the modified program normally reads commands interactively
    when run, you must cause it, when started running for such
    interactive use in the most ordinary way, to print or display an
    announcement including an appropriate copyright notice and a
    notice that there is no warranty (or else, saying that you provide
    a warranty) and that users may redistribute the program under
    these conditions, and telling the user how to view a copy of this
    License.  (Exception: if the Program itself is interactive but
    does not normally print such an announcement, your work based on
    the Program is not required to print an announcement.)

These requirements apply to the modified work as a whole.  If
identifiable sections of that work are not derived from the Program,
and can be reasonably considered independent and separate works in
themselves, then this License, and its terms, do not apply to those
sections when you distribute them as separate works.  But when you
distribute the same sections as part of a whole which is a work based
on the Program, the distribution of the whole must be on the terms of
this License, whose permissions for other licensees extend to the
entire whole, and thus to each and every part regardless of who wrote it.

Thus, it is not the intent of this section to claim rights or contest
your rights to work written entirely by you; rather, the intent is to
exercise the right to control the distribution of derivative or
collective works based on the Program.

In addition, mere aggregation of another work not based on the Program
with the Program (or with a work based on the Program) on a volume of
a storage or distribution medium does not bring the other work under
the scope of this License.

  3. You may copy and distribute the Program (or a work based on it,
under Section 2) in object code or executable form under the terms of
Sections 1 and 2 above provided that you also do one of the following:

    a) Accompany it with the complete corresponding machine-readable
    source code, which must be distributed under the terms of Sections
    1 and 2 above on a medium customarily used for software interchange; or,

    b) Accompany it with a written offer, valid for at least three
    years, to give any third party, for a charge no more than your
    cost of physically performing source distribution, a complete
    machine-readable copy of the corresponding source code, to be
    distributed under the terms of Sections 1 and 2 above on a medium
    customarily used for software interchange; or,

    c) Accompany it with the information you received as to the offer
    to distribute corresponding source code.  (This alternative is
    allowed only for noncommercial distribution and only if you
    received the program in object code or executable form with such
    an offer, in accord with Subsection b above.)

The source code for a work means the preferred form of the work for
making modifications to it.  For an executable work, complete source
code means all the source code for all modules it contains, plus any
associated interface definition files, plus the scripts used to
control compilation and installation of the executable.  However, as a
special exception, the source code distributed need not include
anything that is normally distributed (in either source or binary
form) with the major components (compiler, kernel, and so on) of the
operating system on which the executable runs, unless that component
itself accompanies the executable.

If distribution of executable or object code is made by offering
access to copy from a designated place, then offering equivalent
access to copy the source code from the same place counts as
distribution of the source code, even though third parties are not
compelled to copy the source along with the object code.

  4. You may not copy, modify, sublicense, or distribute the Program
except as expressly provided under this License.  Any attempt
otherwise to copy, modify, sublicense or distribute the Program is
void, and will automatically terminate your rights under this License.
However, parties who have received copies, or rights, from you under
this License will not have their licenses terminated so long as such
parties remain in full compliance.

  5. You are not required to accept this License, since you have not
signed it.  However, nothing else grants you permission to modify or
distribute the Program or its derivative works.  These actions are
prohibited by law if you do not accept this License.  Therefore, by
modifying or distributing the Program (or any work based on the
Program), you indicate your acceptance of this License to do so, and
all its terms and conditions for copying, distributing or modifying
the Program or works based on it.

  6. Each time you redistribute the Program (or any work based on the
Program), the recipient automatically receives a license from the
original licensor to copy, distribute or modify the Program subject to
these terms and conditions.  You may not impose any further
restrictions on the recipients' exercise of the rights granted herein.
You are not responsible for enforcing compliance by third parties to
this License.

  7. If, as a consequence of a court judgment or allegation of patent
infringement or for any other reason (not limited to patent issues),
conditions are imposed on you (whether by court order, agreement or
otherwise) that contradict the conditions of this License, they do not
excuse you from the conditions of this License.  If you cannot
distribute so as to satisfy simultaneously your obligations under this
License and any other pertinent obligations, then as a consequence you
may not distribute the Program at all.  For example, if a patent
license would not permit royalty-free redistribution of the Program by
all those who receive copies directly or indirectly through you, then
the only way you could satisfy both it and this License would be to
refrain entirely from distribution of the Program.

If any portion of this section is held invalid or unenforceable under
any particular circumstance, the balance of the section is intended to
apply and the section as a whole is intended to apply in other
circumstances.

It is not the purpose of this section to induce you to infringe any
patents or other property right claims or to contest validity of any
such claims; this section has the sole purpose of protecting the
integrity of the free software distribution system, which is
implemented by public license practices.  Many people have made
generous contributions to the wide range of software distributed
through that system in reliance on consistent application of that
system; it is up to the author/donor to decide if he or she is willing
to distribute software through any other system and a licensee cannot
impose that choice.

This section is intended to make thoroughly clear what is believed to
be a consequence of the rest of this License.

  8. If the distribution and/or use of the Program is restricted in
certain countries either by patents or by copyrighted interfaces, the
original copyright holder who places the Program under this License
may add an explicit geographical distribution limitation excluding
those countries, so that distribution is permitted only in or among
countries not thus excluded.  In such case, this License incorporates
the limitation as if written in the body of this License.

  9. The Free Software Foundation may publish revised and/or new versions
of the General Public License from time to time.  Such new versions will
be similar in spirit to the present version, but may differ in detail to
address new problems or concerns.

Each version is given a distinguishing version number.  If the Program
specifies a version number of this License which applies to it and "any
later version", you have the option of following the terms and conditions
either of that version or of any later version published by the Free
Software Foundation.  If the Program does not specify a version number of
this License, you may choose any version ever published by the Free Software
Foundation.

  10. If you wish to incorporate parts of the Program into other free
programs whose distribution conditions are different, write to the author
to ask for permission.  For software which is copyrighted by the Free
Software Foundation, write to the Free Software Foundation; we sometimes
make exceptions for this.  Our decision will be guided by the two goals
of preserving the free status of all derivatives of our free software and
of promoting the sharing and reuse of software generally.

                            NO WARRANTY

  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.

                     END OF TERMS AND CONDITIONS

            How to Apply These Terms to Your New Programs

  If you develop a new program, and you want it to be of the greatest
possible use to the public, the best way to achieve this is to make it
free software which everyone can redistribute and change under these terms.

  To do so, attach the following notices to the program.  It is safest
to attach them to the start of each source file to most effectively
convey the exclusion of warranty; and each file should have at least
the "copyright" line and a pointer to where the full notice is found.

    <Ez-Spice program is circuit anlaysis simulation>
    Copyright (C) <2012>  <Jong kwon, Park>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

Also add information on how to contact you by electronic and paper mail.

If the program is interactive, make it output a short notice like this
when it starts in an interactive mode:

    Ez-Spice version 1.0, Copyright (C) 2012 Jong kwon, Park
    Ez-Spice comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
    This is free software, and you are welcome to redistribute it
    under certain conditions; type `show c' for details.

The hypothetical commands `show w' and `show c' should show the appropriate
parts of the General Public License.  Of course, the commands you use may
be called something other than `show w' and `show c'; they could even be
mouse-clicks or menu items--whatever suits your program.

You should also get your employer (if you work as a programmer) or your
school, if any, to sign a "copyright disclaimer" for the program, if
necessary.  Here is a sample; alter the names:

  Yoyodyne, Inc., hereby disclaims all copyright interest in the program
  `Ez-Spice' written by Jong kwon Park.

  <signature of Jong kwon Park>, 06 November 2012
  Jong kwon Park

This General Public License does not permit incorporating your program into
proprietary programs.  If your program is a subroutine library, you may
consider it more useful to permit linking proprietary applications with the
library.  If this is what you want to do, use the GNU Lesser General
Public License instead of this License.

