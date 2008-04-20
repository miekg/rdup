\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{atroff}[2008/04/08 v0.5 ATComputing ATROFF style]

%% Miek Gieben - 2008
%% TODO
%% make \part ATstyle
%% caption title font?? See below

%% Pass all unknown options to memoir
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{memoir}}
\ProcessOptions
\LoadClass{memoir}
\RequirePackage{newcent}
\RequirePackage{amssymb}
\RequirePackage{eurosym}
\RequirePackage{alltt}
\RequirePackage{graphicx}


\makepagestyle{atroff}

\newcommand{\attitle}{\thetitle}
\newlength{\atparindent}
\setlength{\atparindent}{\parindent}
\setlength{\parindent}{0cm}

\makeoddhead{atroff}%
{\sffamily\textsl{\thechapter. \leftmark}}
{}
{\sffamily\textsl{\thepage}}

\makeevenhead{atroff}%
{\sffamily\textsl{\thepage}}
{}
{\sffamily\textsl{\thetitle}}

\makepsmarks{atroff}{%
  \let\@mkboth\markboth
  \def\chaptermark##1{\markboth{##1}{##1}}% % left & right marks
  \def\sectionmark##1{\markright{%          % right mark
    \ifnum \c@secnumdepth>\z@
      \thesection. \%                       % section number
    \fi
    ##1}}
}

% chapter style
\makechapterstyle{atroff}{%
  \renewcommand{\chapterheadstart}{\vspace{2cm}}
  \renewcommand{\printchaptername}{}
  \renewcommand{\chapternamenum}{}
  \renewcommand{\printchapternum}{\raggedleft\LARGE\sffamily\textbf{\thechapter}}
  \renewcommand{\afterchapternum}{\sffamily\textbf{. }}
  \renewcommand{\printchaptertitle}[1]{\raggedleft\LARGE\sffamily\textbf{{##1}}%
    \chaptermark{##1}%
    }
  \renewcommand{\afterchaptertitle}{\vskip\onelineskip \vspace{4.0cm}
  \thispagestyle{empty}}
}

%% part style
\makepagestyle{part}
\renewcommand{\partname}{}
\renewcommand{\partnumfont}{\HUGE\sffamily}
\renewcommand{\parttitlefont}{\HUGE\sffamily}
\renewcommand{\printparttitle}[1]{\vspace{10ex}\parttitlefont #1}
\renewcommand{\printpartnum}{\raggedleft\partnumfont\thepart
  \thispagestyle{empty}
}

%% title page style - defined seperate
\makepagestyle{tatroff}
\makeoddfoot{tatroff}{}{}{}
\makeevenfoot{tatroff}{}{}{}

%% section style
\setsecheadstyle{\raggedright\Large\sffamily\textbf}
  \setbeforesecskip{-\onelineskip}
  \setaftersecskip{\onelineskip}

%% subsection
\setsubsecheadstyle{\sethangfrom{\noindent ##1}\raggedright\sffamily\textbf}
  \setbeforesubsecskip{-\onelineskip}
    \setaftersubsecskip{\onelineskip}

%% subsubsection
\setsubsecheadstyle{\sethangfrom{\noindent ##1}\raggedright\sffamily\textbf}
  \setbeforesubsubsecskip{-\onelineskip}
    \setaftersubsubsecskip{\onelineskip}

%% paragraph
\setparaheadstyle{\sethangfrom{\noindent ##1}\raggedright\sffamily}

%% description
\renewcommand*{\descriptionlabel}[1]{\hspace\labelsep \normalfont\textsl{ #1}}

%% caption for figures
%\makeatletter
\renewcommand{\fnum@figure}{\sffamily{\figurename~\thefigure}}
%\makeatother

%% tableofcontents
\renewcommand{\cftpartfont}{}
\renewcommand{\cftchapterfont}{}

%% set the options for the document
\captiontitlefont{\small}  %% small?
\captionnamefont{}
\pagestyle{atroff}
\chapterstyle{atroff}
\aliaspagestyle{title}{tatroff}

%% enviroments voor atroff
\newenvironment{display}{
  \renewcommand{\verbatim@startline}%
    {\verbatim@line{\hskip\atparindent}}
  \setlength{\baselineskip}{\onelineskip}\verbatim}%
  {\endverbatim
   \vspace{-\baselineskip}\noindent
}

\newenvironment{atleftbar}{%
  \def\FrameCommand{\hskip\atparindent\vrule width 3pt \hspace{10pt}}%
  \MakeFramed {\advance\hsize-\width \FrameRestore}}%
 {\endMakeFramed}

\newenvironment{atblock}{%
  \def\FrameCommand{\hskip\atparindent}%
  \MakeFramed {\advance\hsize-\width \FrameRestore}%
  {\scriptsize $\blacksquare$}
  }%
 { {\scriptsize $\square$}\endMakeFramed}

\newenvironment{atalltt}{%
  \def\FrameCommand{\hskip\atparindent}%
  \MakeFramed {\advance\hsize-\width \FrameRestore}%
  \begin{alltt}
  }%
 {\end{alltt}\endMakeFramed}
