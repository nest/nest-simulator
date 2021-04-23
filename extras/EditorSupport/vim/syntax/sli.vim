" Vim syntax file
" Language: nest SLI
" Maintainer: Ankur Sinha/Nest initiative
" Latest revision: 01 March 2016

if exists("b:current_syntax")
    finish
endif

setlocal softtabstop=2
setlocal cindent shiftwidth=2
setlocal tabstop=2
setlocal expandtab
setlocal cindent

syntax keyword sliTodo TODO XXX FIXME NOTE
hi def link sliTodo        Todo

syn keyword sliKeyword abort abs abs_d abs_i acos add addpath addtotrie allocations and append AppendTo Apply apropos arange area2 area array arrayload ArrayQ ArrayShape arraystore asin assert available backtrace_off backtrace_on begin bind break breakup callback call capacity case cd ceil cerr cin clear cleardict cleardictstack clic clock cloc clonedict close closeinput CompareFiles CompileMath Connect connruledict continue copy CopyFile CopyModel cos count countdictstack counttomark cout Create CreateRDV CreateRNG cst ctermid currentdict currentname cv1d convert 2-dimensional coordinates to 1-dim index cv2d cva cva_d cva_t cvd cvdict cvd_s cv_dv cvnodecollection cvi cvi_s cv_iv cvlit cvn cvo cvs cvs_f cvt_a cvx cycles CyclicValue debug debugoff debugon debug.sli def DeleteFile dexp dict DictQ dictstack Dimensions Directory dirname div Dot1D Dot DoubleQ drand Drop dup2 dup edit elementstates empty end endl endusing environment eof eq_dv eq_iv eq erase Erfc Erf errordict eval EvaluateLiteralInfixes E exch exec ExecFunction ExecMath execstack executive exit exithook exp Export FileNames file FindRoot FiniteQ First FixedPoint Flatten floor flush FoldList Fold forall forallindexed for fork FractionalPart frexp Function gabor_ Gammainc gauss2d_ GaussDiskConv geq GetConnections getc GetDefaults get_d getenv GetGlobalRNG getinterval getline GetMax GetMin GetOption GetOptions getPGRP getPID getPPID get gets GetStatus_dict GetStatus GNUaddhistory GNUreadline good grep gt handleerror HasDifferentMemberQ helpdesk helpindex help iclear ieof ifail ifstream ignore igood in_avail index info info_ds initialize_module Inline insertelement insert inspect Install IntegerPart IntegerQ inv irand iround isatty is_mpi is_threaded is_threaded_with_openmp join joinpath JoinTo kernel keys kill known LambertW0 LambertWm1 LambertW Last LayoutArray ldexp length_a length length_d length_lp length_p length_s leq license LiteralQ ln load LocateFileNames log lookup loop ls lt MakeDirectory Map MapAt MapIndexed MapThread mark MathematicaToSliIndex mathexecutive MatrixQ Max max Mean MemberQ MemoryInfo memory_thisjob_bg memory_thisjob_darwin memory_thisjob MergeDictionary MergeLists message Min min mkfifo mod modeldict modf Most MoveDirectory MoveFile ms2hms mul namespace neg_d neg_i neg neq Nest nest_indirect NestList nest_serial Node noop not npop NumberQ NumProcesses oclear oeof ofsopen ofstream ogood oldgetline ones operandstack Options OptionsDictionary or osstream ostrstream OuterProduct over page pageoutput parsestdin Partition Part path pclockspersec pclocks pcvs pgetrusage pick pipe Pi Plus pop pow ppage pprint prepend print_error PrintNodes PrintNodesToStream print ProcessorName proxynode pstack ptimes put_d putinterval put pwd quit raiseagain raiseerror RandomArray Random RandomSubset Range Rank rdevdict ReadDouble ReadInt readline ReadList ReadModes readPGM Read ReadWord realtime references regcomp regexdict regexec regex_find regex_find_r regex_find_rf regex_find_s regex_find_sf regex_replace RemoveDirectory removeguard repeatany repeat ReplaceOccurrences ReplacePart replace reserve ResetKernel ResetOptions reset RestoreDictionary restoreestack RestoreOptions restoreostack restore Rest resume reverse rngdict rolld roll rollu rot round run SaveDictionary SaveOptions save ScanThread searchfile searchif searchifstream search seed Select SetAcceptableLatency SetDefaults Set SetDirectory SetFakeNumProcesses setguard setNONBLOCK SetOptions setpath setprecision SetStatus_dict SetStatus setverbosity SFWdumpparameters ShowDefaults ShowOptions ShowStatus shpawn shrink signaldict Sign Simulate sin size sleep_i sleep SLIFunctionWrapper SliToMathematicaIndex Sort spawn spikes Split spoon sqr sqrt stack StandardDeviation start statusdict stopped stop StringQ str SubsetQ sub switchdefault switch symbol symbol_s synapsedict SyncProcesses sysexec system systemtime Table Take taskset taskset_thisjob TensorRank tic TimeCommunicationAlltoall TimeCommunicationAlltoall TimeCommunicationAlltoallv TimeCommunicationAlltoallv TimeCommunicationOffgrid TimeCommunication TimeCommunication TimeCommunicationv time Times tmpnam toc token_is token token_s ToLowercase ToMathematicaExpression topinfo_d Total ToUppercase Transpose trie trieinfo trim trunc typebind typeinfo type typestack undef unit_conversion UnitStep usertime using validate values Variance variant verbosity volume_transmitter waitPID wait welcome which who whos writePGM xifstream xor zeros

hi def link sliKeyword     Keyword

syn keyword sliConstant M_INFO M_ERROR M_DEBUG M_WARNING M_STATUS M_FATAL
hi def link sliConstant    Constant

syn match sliUnit '\<mV\>'
syn match sliUnit '\<pF\>'
syn match sliUnit '\<pA\>'
syn match sliUnit '\<ms\>'
syn match sliUnit '\<s\>'
syn match sliUnit '\<Hz\>'
syn match sliUnit '\<nS\>'
hi def link sliUnit      Constant

syn match sliNumber '\d\+'
syn match sliNumber '[-+]\d\+'
syn match sliNumber '[-+]\d\+\.\d*'
syn match sliNumber '[-+]\=\d[[:digit:]]*[eE][\-+]\=\d\+'
syn match sliNumber '\d[[:digit:]]*[eE][\-+]\=\d\+'
syn match sliNumber '[-+]\=\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+'
syn match sliNumber '\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+'
hi def link sliNumber      Constant

" Identifiers
syn match sliIdentifier '/\w\+'
hi def link sliIdentifier   Identifier

" Booleans
syn keyword sliBoolean true false
hi def link sliBoolean     Boolean

" Regions
syn region sliFunc start="{" end="}" fold transparent
hi def link sliFunc        Function

syn region sliDict start="<<" end=">>" fold transparent
hi def link sliDict         Structure

syn region sliGroup start="(" end=")" fold transparent
hi def link sliGroup String

syn match sliComment "%.*$"
hi def link sliComment     Comment
syn region sliCommentBlock start="/\*" end=".*\*/" fold
hi def link sliCommentBlock     Comment

syn keyword sliConditional if ifelse
hi def link sliConditional  Conditional

syn keyword sliRepeat forall repeat
hi def link sliRepeat Repeat

" Finishing touches
let b:current_syntax = "sli"
