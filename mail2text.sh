#!/usr/bin/bash
# From https://unix.stackexchange.com/a/773290/800708
# 
declare mail_file output_file output_dir append_prefix\
        section_ID section_mimeType section_charset\
        choosen_Indices section_ID skip_RegExp section_charset conv_charset="UTF-8"\
        command_name option report help\
        stderrC=$'\e[38;2;240;80;0m' indexC=$'\e[38;2;0;200;0m' previewC=$'\e[38;2;200;200;0m' 
declare -a sections contents opt_subArgs
declare -A skip_type=(["typeID"]=1 ["invert"]=0 ["ignoreCase"]=1 ["0"]="typeID" ["1"]="invert" ["2"]="ignoreCase")
declare -i i sections_L preview_chars=200 convHTMLToText=1 doConvCharset dontOverWrite=0 append_type append_digits=4 main_exit_code=-1 HTMLtoText_width=79

command_name=$(readlink -f "$0"); command_name=${command_name##*/}

if [[ $1 == "--help" ]]; then
   help=$(cat << EOF
\e[38;2;123;183;51m\e[1;4mUsage:\e[39m\e[22;24;4:0m $command_name [\e[38;2;240;240;0m\e[3mOPTION...\e[39m\e[23m] [\e[38;2;240;240;0m\e[3mMAIL-FILE\e[39m\e[23m] [\e[38;2;240;240;0m\e[3mOUTPUT-FILE\e[39m\e[23m]

Extracts mail mimeType sections of text/plain or text/html and converts text/html to text/plain.
If more sections are found, let you choose the sections - \e[3mseparator ";" (x;y;...)\e[23m
If no OUTPUT-FILE is overhanded, writes to STDOUT.
\e[49m
\e[38;2;123;183;51m\e[1;4mDepends On:\e[39m\e[22;24;4:0m commands - reformime, html2text

\e[38;2;123;183;51m\e[1;4mOptions:\e[39m\e[22;24;4:0m
The option argument separator ':' can be escaped with '\\\\'.
Dont escape '\\\\' with '\\\\\\\\' to prohibit separator escaping and keep the literal meaning, it will count as double '\\\\''\\\\'. Use double quotes on part!
Double quotes will be removed at start and end position of splitted part. If option argument ends with '\\\\', use double quotes on the last part.

\e[3mArgument examples: $command_name -a \e[38;2;240;240;0m\e[3m'\e[39m1\e[38;2;240;240;0m\e[3m:\e[39mapp\\\\:en\\\\:d me\e[38;2;240;240;0m\e[3m:\e[39m\e[23m6\e[38;2;240;240;0m\e[3m'\e[39m or -a \e[38;2;240;240;0m\e[3m'\e[39m1\e[38;2;240;240;0m\e[3m:\e[39m"append me\\\\"\e[38;2;240;240;0m\e[3m:\e[39m6\e[38;2;240;240;0m\e[3m'\e[39m or \e[38;2;240;240;0m\e[3m'\e[39m1\e[38;2;240;240;0m\e[3m:\e[39m"app\\\\\\\\en\\\\\\\\:d me\\\\\\\\"\e[38;2;240;240;0m\e[3m'\e[39m 
Argument parts: '1', 'app:en:d me', '6'  -  '1', 'append me\\\\', '6'  -  1, 'app\\\\\\\\en\\\\:d me\\\\\\\\'\e[39m\e[23m

   -\e[38;2;145;246;214ma\e[39m \e[38;2;240;240;0m\e[3mappend\e[39m\e[23m        \e[38;2;103;134;250mSTR\e[39m ... do not overwrite existing file! - [\e[38;2;240;240;0m\e[3mtypeID\e[39m\e[23m \e[38;2;103;134;250mINT\e[39m]:[\e[38;2;240;240;0m\e[3mprefix\e[39m\e[23m \e[38;2;103;134;250mSTR\e[39m]:[\e[38;2;240;240;0m\e[3mdigits\e[39m\e[23m \e[38;2;103;134;250mINT\e[39m]
                            \e[38;2;240;240;0m\e[3mtypeID\e[39m\e[23m \e[38;2;103;134;250mINT\e[39m [1-2]
                               1 ... append data to file using the defined prefix.
                               2 ... change filename by appending a continuing number at end,
                                     respecting a filename extension. (filenameXXXX.ext)
                            \e[38;2;240;240;0m\e[3mprefix\e[39m\e[23m \e[38;2;103;134;250mSTR\e[39m optional (Standard: "")
                               prefix for appending mail section content.
                            \e[38;2;240;240;0m\e[3mdigits\e[39m\e[23m \e[38;2;103;134;250mINT\e[39m optional (Standard: 4)
                               minimum digits of continuing number
   -\e[38;2;145;246;214mc\e[39m \e[38;2;240;240;0m\e[3mconv_charset\e[39m\e[23m  \e[38;2;103;134;250mSTR\e[39m ... convert section content to this charset. (Standard: UTF-8)
                            for supported charsets execute "iconv -l". The empty string ("") means keep charset.
   -\e[38;2;145;246;214md\e[39m                       do not convert text/html to text/plain!
   -\e[38;2;145;246;214mp\e[39m \e[38;2;240;240;0m\e[3mpreview_chars\e[39m\e[23m \e[38;2;103;134;250mINT\e[39m ... character length of section content preview, if more sections are found! (Standard: 200)
   -\e[38;2;145;246;214mr\e[39m \e[38;2;240;240;0m\e[3mskip_RegExp\e[39m\e[23m   \e[38;2;103;134;250mSTR\e[39m ... GAWK regular expression pattern for skipping data in mail section content.
   -\e[38;2;145;246;214mt\e[39m \e[38;2;240;240;0m\e[3mskip_type\e[39m\e[23m     \e[38;2;103;134;250mSTR\e[39m ... define skip type - [\e[38;2;240;240;0m\e[3mtypeID\e[39m\e[23m \e[38;2;103;134;250mINT\e[39m]:[\e[38;2;240;240;0m\e[3minvert\e[39m\e[23m \e[38;2;103;134;250mBOOL\e[39m]:[\e[38;2;240;240;0m\e[3mignoreCase\e[39m\e[23m \e[38;2;103;134;250mBOOL\e[39m]
                            \e[38;2;240;240;0m\e[3mtypeID\e[39m\e[23m \e[38;2;103;134;250mINT\e[39m [1-2] (Standard: 1)
                               1 ... skip from first-match-start-position to section-content-end
                                     or from section-content-start to first-match-start-position. (inverse skipping)
                               2 ... remove globally matches from section content
                                     or print only globally matches from section content.         (inverse skipping)
                            \e[38;2;240;240;0m\e[3minvert \e[38;2;103;134;250mBOOL\e[39m [0 or >0] optional (Standard: 0)
                               >0 ... invert skipping
                            \e[38;2;240;240;0m\e[3mignoreCase\e[39m\e[23m \e[38;2;103;134;250mBOOL\e[39m [0 or >0] optional (Standard: 1)
                               >0 ... ignore case
   -\e[38;2;145;246;214mw\e[39m \e[38;2;240;240;0m\e[3mHTML_width\e[39m\e[23m    \e[38;2;103;134;250mINT\e[39m ... By default, html2text formats the HTML documents for a screen width of 79 characters
   
GAWK RegExp description - https://www.gnu.org/software/gawk/manual/html_node/Regexp.html
For mimeType "text/html" the pattern search is executed on HTML, before converting to "text/plain".

\e[38;2;123;183;51m\e[1;4mExamples:\e[39m\e[22;24;4:0m
   $command_name -p 300 -r 'hello' -t 1:1 [\e[38;2;240;240;0m\e[3mMAIL-FILE\e[39m\e[23m]
      sets preview characters to 300 - A preview will be shown, if more sections are found.
      sets a RegExp, which will find "hello" in content data,
      sets typeID to 1 and invert to 1 - the data from section-content-start to first-match-start of "hello" will be skipped.
   
   $command_name -r 'he+llo' -t 2 [\e[38;2;240;240;0m\e[3mMAIL-FILE\e[39m\e[23m]
      sets a RegExp, which will find "hello", "heello", "heeello", ... in content data,
      sets typeID to 2 - the matches on content data will be removed globally.
   
   $command_name -r '[^\\\\n]*hello[^\\\\n]\\\\n' -t 2:1 [\e[38;2;240;240;0m\e[3mMAIL-FILE\e[39m\e[23m]
      sets a RegExp, which will match a line with hello,
      sets typeID to 2 and invert to 1 - only the globally matches on content data will be printed.
      
\e[48;2;245;0;10mBug reports to: https://unix.stackexchange.com/questions/773264/convert-e-mail-to-text-reformime\e[49m
EOF
); printf "%b" "$help" | less -rXP "Hit 'q' to exit ---"; exit
fi 

str_split_ES ()
   {
   local -n LOC_ARR=$2; local -i LOC_i=${#LOC_ARR[*]}
   local LOC_part LOC_partCarry=""
   
   for LOC_part in $1$IFS; do
      if [[ $LOC_part && ${LOC_part: -1} == '\' ]]; then
         LOC_partCarry+=${LOC_part:0: -1}$IFS
      else
         LOC_part=${LOC_partCarry}${LOC_part%'"'};
         LOC_ARR[LOC_i++]=${LOC_part#'"'}
         LOC_partCarry=""
      fi
   done
   [[ -z ${LOC_ARR[--LOC_i]} ]] && unset "LOC_ARR[LOC_i]";
   }
   
echo -en "$stderrC" >/dev/tty; set -o noglob
while getopts "a:c:dp:r:t:w:" "option"; do
   case $option in
      "a") dontOverWrite=1; opt_subArgs=(); IFS=":" str_split_ES "$OPTARG" "opt_subArgs"; append_type=${opt_subArgs[0]}; append_prefix=${opt_subArgs[1]}; append_digits=${opt_subArgs[2]:-$append_digits};;
      "c") conv_charset=$OPTARG;;
      "d") convHTMLToText=0;;
      "p") preview_chars=$OPTARG;;
      "r") skip_RegExp=$OPTARG;;
      "t") opt_subArgs=(); IFS=":" str_split_ES "$OPTARG" "opt_subArgs"; for ((i=0;i<${#opt_subArgs[*]};i++)); do skip_type[${skip_type[$i]}]=${opt_subArgs[i]}; done;;
      "w") HTMLtoText_width=$OPTARG;;
      "?") exit 2;;
   esac
done
shift $((OPTIND-1)); [[ ${#*} -gt 2 ]] && { echo "$command_name - too many arguments after recognized options - allowed [MAIL-FILE] [OUTPUT-FILE]- check your options!" 1>&2 ; exit 2; } 
mail_file=$1; output_file=$2;

if [[ $output_file ]]; then
   output_dir=$(dirname "$output_file")'/'
   [[ ! -d $output_dir ]] && mkdir -p "$output_dir"
fi
   
skip_text ()
   {
   awk -v skip_RegExp="$skip_RegExp" ' 
   BEGIN {
      RS="^$"; FS=""; skip_type='${skip_type[typeID]}'; skip_invert='${skip_type[invert]}'; skip_ic='${skip_type[ignoreCase]}';
      section_mimeType="'"$section_mimeType"'"; section_charset="'"$section_charset"'"; convHTMLToText='$convHTMLToText'; conv_charset="'"$conv_charset"'";
   }
   { if (section_mimeType=="text/html")
         {
         IGNORECASE=1;
         if (conv_charset && conv_charset != section_charset)
            {
            gsub(/<meta *http-equiv[^>]*>/, "");
            sub(/<head>/, "<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=" conv_charset "\">");
            }
         else if (convHTMLToText && match($0, /<meta *http-equiv[^>]*charset/)==0)
            sub(/<head>/, "<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=" section_charset "\">");
         }
     
     if (skip_RegExp=="") { printf "%s", $0; exit }
     IGNORECASE=skip_ic;
     switch (skip_type)
       {
       case 1: match($0, skip_RegExp);
               if (RSTART>0) 
                  { 
                  if (skip_invert) printf "%s", substr($0, RSTART, length($0)-RSTART+1);
                  else printf "%s", substr($0, 1, RSTART-1);
                  } 
               else printf "%s", $0;
               break;
       case 2: if (skip_invert) 
                  { 
                  text=$0; while (match(text, skip_RegExp))
                     { 
                     printf "%s", substr(text, RSTART, RLENGTH);
                     text=substr(text, RSTART+RLENGTH, length(text)-RS-widthTART-RLENGTH+1);
                     }
                  } 
               else { gsub(skip_RegExp, ""); printf "%s", $0 }
               break;
       }
   }'   
   }

convert_charset ()
   {
   if ((doConvCharset)); then echo -n "${contents[$1]}" | iconv -f "$section_charset" -t "$conv_charset"
   else echo -n "${contents[$1]}"
   fi
   }
   
print_sectionContent ()
   {
   local conv_report="" data filename filename_wext filename_ext append_number_STR
   local -i i pipe_failExit append_number
   local -a pipe_exits
   
   ((main_exit_code<0)) && main_exit_code=0;
   
   section_charset=${sections[$1]#*charset:}; section_charset=${section_charset%%$'\n'*}
   section_charset=${section_charset//['"' ]/}; section_charset=${section_charset^^};   
   if [[ $conv_charset && $conv_charset != "$section_charset" ]]; then
      doConvCharset=1; conv_report="charset:$conv_charset"
   else doConvCharset=0
   fi
   
   if [[ ${sections[$1]} == *text/html* ]]; then
      section_mimeType="text/html"
      if ((convHTMLToText)); then
         conv_report="text/plain-"$conv_report
         #data=$(convert_charset $1 |  skip_text | html2text -width "$HTMLtoText_width" | iconv -c -f "${conv_charset:-$section_charset}" -t "${conv_charset:-$section_charset}"; echo -n ";${PIPESTATUS[*]}") 
         data=$(convert_charset $1 |  skip_text | iconv -c -f "${conv_charset:-$section_charset}" -t "${conv_charset:-$section_charset}"; echo -n ";${PIPESTATUS[*]}") 
      else
         data=$(convert_charset $1 | skip_text; echo -n ";${PIPESTATUS[*]}")
      fi
   else
      section_mimeType="text/plain"
      data=$(convert_charset $1 | skip_text; echo -n ";${PIPESTATUS[*]}")
   fi

   : "${data%;*}"; i=${#_}; pipe_exits=(${data:i+1}); data=${data:0:i}
   for ((i=0;i<${#pipe_exits[*]};i++)); do ((pipe_failExit=pipe_failExit | pipe_exits[i])); done
   
   if ((pipe_failExit)); then
      report+="pipe failed (command in pipe, exit status >0) - section [$(($1+1))] of mimeType ${section_mimeType}-charset:$section_charset!"$'\n'
      return 4
   else 
      if [[ -z $output_file ]]; then echo -ne "\e[1;0m" >/dev/tty; echo -n "$data"; echo -en "$stderrC" >/dev/tty;
      elif [[ -f "$output_file" ]]; then
         if ((dontOverWrite)); then
            case $append_type in
               1) [[ -z $append_prefix ]] && echo -n "$data" >> "$output_file" || echo -n "$append_prefix$data" >> "$output_file";;
               2) filename=${output_file##*/}; filename_wext=${filename%"."*}; filename_ext=${filename:${#filename_wext}}
                  filename=${filename_wext%%*([0-9])};
                  append_number_STR=${filename_wext:${#filename}}; append_number=${append_number_STR##*([0])};
                  while :; do
                     ((append_number++))
                     printf -v append_number_STR "%0.${append_digits}d" "$append_number"
                     output_file="$output_dir$filename$append_number_STR$filename_ext"
                     [[ ! -f $output_file ]] && break;
                  done
                  echo -n "$data" > "$output_file";;
               *) echo "append typeID not found - stopped executing at mail-file < ${mail_file}" 1>&2 ; exit 255;;
            esac
         else echo -n "$data" >"$output_file"
         fi
      else echo -n "$data" >"$output_file"
      fi
      if (($?==0)); then
         if [[ $conv_report ]]; then
            [[ ${conv_report:${#conv_report}-1:1} == "-" ]] && conv_report=${conv_report:0:-1}
            conv_report="converted to $conv_report "
         fi
         report+="section [$(($1+1))] of mimeType ${section_mimeType}-charset:$section_charset ${conv_report}written to ${output_file:-stdout}!"$'\n'
      else
         report+="write error - wasnt able to write section [$(($1+1))] of mimeType ${section_mimeType}-charset:$section_charset!"$'\n'
         return 3
      fi
   fi
   }

[[ -z $mail_file || ! -f $mail_file ]] && { echo "$command_name - mail-file < ${mail_file}"$'\n'"no mail file found!" 1>&2; exit 127; }
IFS=$'\x1e'
sections=($(reformime -i <"$mail_file"| { (($?)) && exit; grep -zoP "section[^\n]*\n[^\n]*text/(plain|html)\n([^\n]*\n[^\n])*harset:[^\n]*"; } | sed "s/\x00/\x1e/g"))
sections_L=${#sections[*]}
shopt -s extglob

for ((i=0;i<$sections_L;i++)); do
   section_ID=${sections[i]%%$'\n'*}; section_ID=${section_ID#*: }
   contents[i]=$(reformime -e -s "$section_ID" < "$mail_file")
done

case $sections_L in
   0) echo "$command_name - mail-file < ${mail_file}"$'\n'"no mimeType text/plain or text/html sections found!" 1>&2 ; exit 5;;
   1) IFS=":" print_sectionContent 0;;
   *) echo $'\e[1;0mFound sections of mimeType text/(plain|html):\n-\e[44b' >/dev/tty
      for ((i=0;i<$sections_L;i++)); do
        echo -e "\n[$indexC$((i+1))\e[1;0m]${sections[i]}\n$previewC${contents[i]:0:$preview_chars}\e[1;0m" >/dev/tty
      done
      echo -ne "\nChoose section(s) [x] :\e7" >/dev/tty
      while [[ -z $choosen_Indices || $choosen_Indices != +([0-9]*(;)) ]]; do echo -n $'\e8\e[0K' >/dev/tty; read choosen_Indices; done
      echo -en "$stderrC" >/dev/tty; IFS=";"; for i in $choosen_Indices; do ((i>0 && i<=sections_L)) && { IFS=":" print_sectionContent $((i-1)); ((main_exit_code+=$?)); }; done ;;
esac

((main_exit_code<0)) && { report="try to enter a section index, that exists!\n"; main_exit_code=6; }
echo -en "\n$command_name - mail-file < ${mail_file}\n$report" 1>&2; echo -en "\e[1;0m" >/dev/tty; exit $main_exit_code
