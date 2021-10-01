#! /usr/bin/env bash

printf 'Updating repos\n';
git subtree pull --prefix mpy/paren_hl https://github.com/kammerdienerb/yed-paren_hl master --squash;
git subtree pull --prefix mpy/scroll_buffer https://github.com/kammerdienerb/yed-scroll_buffer master --squash;
git subtree pull --prefix mpy/man https://github.com/kammerdienerb/yed-man master --squash;
git subtree pull --prefix mpy/shell_run https://github.com/kammerdienerb/yed-shell_run master --squash;
git subtree pull --prefix mpy/status_bar_clock https://github.com/kammerdienerb/yed-status_bar_clock master --squash;
git subtree pull --prefix mpy/style_pack https://github.com/kammerdienerb/yed-style_pack master --squash;
git subtree pull --prefix mpy/style_picker https://github.com/kammerdienerb/yed-style_picker/ master --squash;
git subtree pull --prefix mpy/style_use_term_bg https://github.com/kammerdienerb/yed-style_use_term_bg master --squash;
git subtree pull --prefix mpy/yedrc https://github.com/kammerdienerb/yed-yedrc master --squash;
git subtree pull --prefix mpy/calc https://github.com/kammerdienerb/yed-calc master --squash;
git subtree pull --prefix mpy/comment https://github.com/kammerdienerb/yed-comment master --squash;
git subtree pull --prefix mpy/completer https://github.com/kammerdienerb/yed-completer master --squash;
git subtree pull --prefix mpy/ctags https://github.com/kammerdienerb/yed-ctags master --squash;
git subtree pull --prefix mpy/cursor_word_hl https://github.com/kammerdienerb/yed-cursor_word_hl master --squash;
git subtree pull --prefix mpy/find_bracket https://github.com/mickidymick/find_bracket master --squash;
git subtree pull --prefix mpy/find_bracket https://github.com/mickidymick/find_bracket main --squash;
git subtree pull --prefix mpy/find_file https://github.com/kammerdienerb/yed-find_file master --squash;
git subtree pull --prefix mpy/grep https://github.com/kammerdienerb/yed-grep master --squash;
git subtree pull --prefix mpy/hook https://github.com/kammerdienerb/yed-hook master --squash;
git subtree pull --prefix mpy/indent_c https://github.com/kammerdienerb/yed-indent_c master --squash;
git subtree pull --prefix mpy/jump_stack https://github.com/kammerdienerb/yed-jump_stack master --squash;
git subtree pull --prefix mpy/line-numbers https://github.com/kammerdienerb/yed-line_numbers master --squash
git subtree pull --prefix mpy/log_hl https://github.com/kammerdienerb/yed-log_hl master --squash
git subtree pull --prefix mpy/autotrim https://github.com/kammerdienerb/yed-autotrim master --squash
git subtree pull --prefix mpy/brace_hl https://github.com/kammerdienerb/yed-brace_hl master --squash
git subtree pull --prefix mpy/auto_paren https://github.com/mickidymick/auto_paren main --squash
git subtree pull --prefix mpy/lang/c https://github.com/kammerdienerb/yed-lang-c master --squash
git subtree pull --prefix mpy/lang/conf https://github.com/kammerdienerb/yed-lang-conf master --squash
git subtree pull --prefix mpy/lang/cpp https://github.com/kammerdienerb/yed-lang-cpp master --squash
git subtree pull --prefix mpy/lang/glsl https://github.com/kammerdienerb/yed-lang-glsl master --squash
git subtree pull --prefix mpy/lang/jgraph https://github.com/kammerdienerb/yed-lang-jgraph master --squash
git subtree pull --prefix mpy/lang/latex https://github.com/kammerdienerb/yed-lang-latex master --squash
git subtree pull --prefix mpy/lang/make https://github.com/kammerdienerb/yed-lang-make master --squash
git subtree pull --prefix mpy/lang/python https://github.com/kammerdienerb/yed-lang-python master --squash
git subtree pull --prefix mpy/lang/sh https://github.com/kammerdienerb/yed-lang-sh master --squash
git subtree pull --prefix mpy/lang/yedrc https://github.com/kammerdienerb/yed-lang-yedrc master --squash
git subtree pull --prefix mpy/lang/syntax/c https://github.com/kammerdienerb/yed-lang-syntax-c master --squash
git subtree pull --prefix mpy/lang/syntax/conf https://github.com/kammerdienerb/yed-lang-syntax-conf master --squash
git subtree pull --prefix mpy/lang/syntax/cpp https://github.com/kammerdienerb/yed-lang-syntax-cpp master --squash
git subtree pull --prefix mpy/lang/syntax/glsl https://github.com/kammerdienerb/yed-lang-syntax-glsl master --squash
git subtree pull --prefix mpy/lang/syntax/jgraph https://github.com/kammerdienerb/yed-lang-syntax-jgraph master --squash
git subtree pull --prefix mpy/lang/syntax/latex https://github.com/kammerdienerb/yed-lang-syntax-latex master --squash
#git subtree pull --prefix mpy/lang/syntax/make https://github.com/kammerdienerb/yed-lang-syntax-make master --squash
git subtree pull --prefix mpy/lang/syntax/python https://github.com/kammerdienerb/yed-lang-syntax-python master --squash
git subtree pull --prefix mpy/lang/syntax/sh https://github.com/kammerdienerb/yed-lang-syntax-sh master --squash
git subtree pull --prefix mpy/lang/syntax/yedrc https://github.com/kammerdienerb/yed-lang-syntax-yedrc master --squash
git subtree pull --prefix mpy/lang/tools/latex https://github.com/kammerdienerb/yed-lang-tools-latex master --squash

printf 'Copying plugin man pages\n';
printf '%s\0' mpy/*/ | xargs -0 -L1 bash -c 'cd -- "$1" && cp *.7 ../../mpyconf/man/man7/.' _;
printf 'Copying lang man pages\n';
printf '%s\0' mpy/lang/*/ | xargs -0 -L1 bash -c 'cd -- "$1" && cp *.7 ../../../mpyconf/man/man7/.' _;
printf 'Copying lang/syntax man pages\n';
printf '%s\0' mpy/lang/syntax/*/ | xargs -0 -L1 bash -c 'cd -- "$1" && cp *.7 ../../../../mpyconf/man/man7/.' _;

printf 'Building plugins\n';
printf '%s\0' mpy/*/ | xargs -0 -L1 bash -c 'cd -- "$1" && ./build.sh && cp *.so ../../userconf/plugins/.' _

printf 'Building lang plugins\n';
printf '%s\0' mpy/lang/*/ | xargs -0 -L1 bash -c 'cd -- "$1" && ./build.sh && cp *.so ../../../userconf/plugins/lang/.' _

printf 'Building syntax plugins\n';
printf '%s\0' mpy/lang/syntax/*/ | xargs -0 -L1 bash -c 'cd -- "$1" && ./build.sh && cp *.so ../../../../userconf/plugins/lang/syntax/.' _

printf 'Final touches...\n';
