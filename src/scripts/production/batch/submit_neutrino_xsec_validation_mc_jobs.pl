#!/usr/bin/perl

#---------------------------------------------------------------------------------------------------------------------
# Submit jobs to generate all data needed for validating GENIE's cross section model 
# The generated data can be fed into GENIE's gvld_nu_xsec validation app
#
# Syntax:
#   perl submit_neutrino_xsec_validation_mc_jobs.pl <options>
#
# Options:
#    --version       : GENIE version number
#   [--nsubruns]     : number of subruns per run, default: 1
#   [--offset]       : subrun offset (for augmenting existing sample), default: 0
#   [--arch]         : <SL4_32bit, SL5_64bit>, default: SL5_64bit
#   [--production]   : production name, default: <version>
#   [--cycle]        : cycle in current production, default: 01
#   [--use-valgrind] : default: off
#   [--batch-system] : <PBS, LSF>, default: PBS
#   [--queue]        : default: prod
#   [--softw-topdir] : default: /opt/ppd/t2k/softw/GENIE
#
# Tested at the RAL/PPD Tier2 PBS batch farm.
#
# Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
# STFC, Rutherford Appleton Lab
#---------------------------------------------------------------------------------------------------------------------
#
# EVENT SAMPLES:
# The following samples will be submitted.
# The output GHEP event trees will be analyzed and converted to GST summary trees.
# The validation apps use the samples to decompose the inclusive cross section
# to cross section for various exclusive channels
#......................................................................
# run number      |  init state      | energy   | processes
#                 |                  | (GeV)    | enabled
#......................................................................
#
# 1000xx          | numu    + n      | 0.1-120. | all
# 1100xx          | numu    + p      | 0.1-120. | all
# 1200xx          | numubar + n      | 0.1-120. | all
# 1300xx          | numubar + p      | 0.1-120. | all
# 2010xx          | numu    + 12C    | 0.1-120. | all
#
# xx : Run ID, 01-99, 100k events each
#......................................................................
#
# CROSS SECTIONS:
# A job will be submitted to run GENIE's gspl2root utility, extract
# cross sections (from the pre-computed XML cross section spline file) 
# and save them into a single ROOT file (xsec.root).
# The file will contain the following folders (TDirectories):
# - nu_mu_n
# - nu_mu_H1
# - nu_mu_bar_n
# - nu_mu_bar_H1
# - nu_mu_C12
# - nu_mu_bar_C12
# - nu_mu_Ne20
# - nu_mu_bar_Ne20
# - nu_mu_Al27
# - nu_mu_Si30     (Si30: proxy for freon (average A=30), used for coherent pi production plots only)
# - nu_mu_bar_Si30 ( -//- )
# Each folder will contain graphs for all cross sections for the given initial state.
# Expand as required if more published data are inluded in this GENIE physics benchmark test.
#

use File::Path;

# inputs
#
$iarg=0;
foreach (@ARGV) {
  if($_ eq '--version')       { $genie_version = $ARGV[$iarg+1]; }
  if($_ eq '--nsubruns')      { $nsubruns      = $ARGV[$iarg+1]; }
  if($_ eq '--offset')        { $offset        = $ARGV[$iarg+1]; }
  if($_ eq '--arch')          { $arch          = $ARGV[$iarg+1]; }
  if($_ eq '--production')    { $production    = $ARGV[$iarg+1]; }
  if($_ eq '--cycle')         { $cycle         = $ARGV[$iarg+1]; }
  if($_ eq '--use-valgrind')  { $use_valgrind  = $ARGV[$iarg+1]; }
  if($_ eq '--batch-system')  { $batch_system  = $ARGV[$iarg+1]; }
  if($_ eq '--queue')         { $queue         = $ARGV[$iarg+1]; }
  if($_ eq '--softw-topdir')  { $softw_topdir  = $ARGV[$iarg+1]; }  
  $iarg++;
}
die("** Aborting [Undefined GENIE version. Use the --version option]")
unless defined $genie_version;

$nsubruns       = 1                          unless defined $nsubruns;
$offset         = 0                          unless defined $offset;
$use_valgrind   = 0                          unless defined $use_valgrind;
$arch           = "SL5_64bit"                unless defined $arch;
$production     = "$genie_version"           unless defined $production;
$cycle          = "01"                       unless defined $cycle;
$batch_system   = "PBS"                      unless defined $batch_system;
$queue          = "prod"                     unless defined $queue;
$softw_topdir   = "/opt/ppd/t2k/softw/GENIE" unless defined $softw_topdir;
if($batch_system eq 'PBS') {$time_limit     = "60:00:00"; } else {$time_limit     = "60:00";}
$genie_setup    = "$softw_topdir/builds/$arch/$genie_version-setup";
$jobs_dir       = "$softw_topdir/scratch/vld\_xsec-$production\_$cycle";
$xspl_file      = "$softw_topdir/data/job_inputs/xspl/gxspl-vA-$genie_version.xml";
$mcseed         = 210921029;
$nev_per_subrun = 100000;

# inputs for event generation jobs
%evg_nupdg_hash = ( 
  '1000' =>   '14',
  '1100' =>   '14',
  '1200' =>  '-14',
  '1300' =>  '-14',
  '2010' =>   '14'
);
%evg_tgtpdg_hash = ( 
  '1000' =>  '1000000010',
  '1100' =>  '1000010010',
  '1200' =>  '1000000010',
  '1300' =>  '1000010010',
  '2010' =>  '1000060120'
);

%evg_energy_hash = ( 
  '1000' =>  '0.1,120.0',
  '1100' =>  '0.1,120.0',
  '1200' =>  '0.1,120.0',
  '1300' =>  '0.1,120.0',
  '2010' =>  '0.1,120.0'
);
%evg_gevgl_hash = ( 
  '1000' =>  'Default',
  '1100' =>  'Default',
  '1200' =>  'Default',
  '1300' =>  'Default',
  '2010' =>  'Default'
);
%evg_fluxopt_hash = ( 
  '1000' =>  '-f "1/x"',
  '1100' =>  '-f "1/x"',
  '1200' =>  '-f "1/x"',
  '1300' =>  '-f "1/x"',
  '2010' =>  '-f "1/x"'
);

# inputs for xsec calculation jobs
%xsec_nupdg_hash = ( 
  '1' =>   '14',
  '2' =>   '14',
  '3' =>  '-14',
  '4' =>  '-14',
  '5' =>   '14',
  '6' =>  '-14',
  '7' =>   '14',
  '8' =>   '14',
  '9' =>  '-14',
 '10' =>   '14',
 '11' =>  '-14'
);
%xsec_tgtpdg_hash = ( 
  '1' =>  '1000000010',
  '2' =>  '1000010010',
  '3' =>  '1000000010',
  '4' =>  '1000010010',
  '5' =>  '1000100200', # Ne20
  '6' =>  '1000100200', # Ne20
  '7' =>  '1000130270', # Al27
  '8' =>  '1000140300', # Si30, proxy for freon (average A=30), used for coherent pi production plots only
  '9' =>  '1000140300', # Si30, proxy for freon (average A=30), used for coherent pi production plots only
 '10' =>  '1000060120', # C12
 '11' =>  '1000060120'  # C12
);

# make the jobs directory
#
mkpath ($jobs_dir, {verbose => 1, mode=>0777});

#
# submit event generation jobs
#
for my $curr_runnu (keys %evg_gevgl_hash)  {

 # uncomment if you want to include a cmd line input to specify specific runs (and fill-in $runnu)
 # if($runnu=~m/$curr_runnu/ || $runnu eq "all") {

    print "** submitting event generation run: $curr_runnu \n";

    #
    # get runnu-dependent info
    #
    $nu      = $evg_nupdg_hash   {$curr_runnu};
    $tgt     = $evg_tgtpdg_hash  {$curr_runnu};
    $en      = $evg_energy_hash  {$curr_runnu};
    $gevgl   = $evg_gevgl_hash   {$curr_runnu};
    $fluxopt = $evg_fluxopt_hash {$curr_runnu};

    # submit subruns
    for($isubrun = 0; $isubrun < $nsubruns; $isubrun++) {

       $curr_subrunnu = 100 * $curr_runnu + $isubrun + $offset;
       $fntemplate    = "$jobs_dir/xsecvld-$curr_subrunnu";
       $curr_seed     = $mcseed + $isubrun + $offset;
       $grep_pipe     = "grep -B 100 -A 30 -i \"warn\\|error\\|fatal\"";
       $valgrind_cmd  = "valgrind --tool=memcheck --error-limit=no --leak-check=yes --show-reachable=yes";
       $evgen_opt     = "-n $nev_per_subrun -e $en -p $nu -t $tgt $fluxopt -r $curr_subrunnu --seed $curr_seed --cross-sections $xspl_file --event-generator-list $gevgl";
       $evgen_cmd     = "gevgen $evgen_opt | $grep_pipe &> $fntemplate.evgen.log";
       $conv_cmd      = "gntpc -f gst -i gntp.$curr_subrunnu.ghep.root | $grep_pipe &> $fntemplate.conv.log";

       print "@@ exec: $evgen_cmd \n";

       #
       # submit
       #

       # PBS case
       if($batch_system eq 'PBS') {  
           $batch_script  = "$fntemplate.pbs";
           open(PBS, ">$batch_script") or die("Can not create the PBS batch script");
           print PBS "#!/bin/bash \n";
           print PBS "#PBS -l cput=$time_limit \n";
           print PBS "#PBS -o $fntemplate.pbsout.log \n";
           print PBS "#PBS -e $fntemplate.pbserr.log \n";
           print PBS "source $genie_setup \n"; 
           print PBS "cd $jobs_dir \n";
           print PBS "$evgen_cmd \n";
           print PBS "$conv_cmd \n";
           close(PBS);
           `qsub -q $queue $batch_script`;
       } #PBS

       # LSF case
       if($batch_system eq 'LSF') {  
           $batch_script  = "$fntemplate.sh";
           open(LSF, ">$batch_script") or die("Can not create the LSF batch script");
           print LSF "#!/bin/bash \n";
           print LSF "#BSUB-q $queue \n";
           print LSF "#BSUB-c $time_limit \n";
           print LSF "#BSUB-o $fntemplate.lsfout.log \n";
           print LSF "#BSUB-e $fntemplate.lsferr.log \n";
           print LSF "source $genie_setup \n"; 
           print LSF "cd $jobs_dir \n";
           print LSF "$evgen_cmd \n";
           print LSF "$conv_cmd \n";
           close(LSF);
           `bsub < $batch_script`;
       } #LSF

    } # loop over subruns
 # } #checking whether to submit current run
} # loop over runs


#
# submit job to generate the cross section ROOT file
#
{
   $gspl2root_cmd = "";
   for my $curr_init_state (keys %xsec_nupdg_hash)  {
      $nu  = $xsec_nupdg_hash   {$curr_init_state};
      $tgt = $xsec_tgtpdg_hash  {$curr_init_state};
      $gspl2root_cmd = "$gspl2root_cmd \ngspl2root -p $nu -t $tgt -f $xspl_file -o xsec.root";
   }
   print "@@@ exec: $gspl2root_cmd \n";

   $fntemplate = "$jobs_dir/xsecvld-xsec";

   # PBS case
   if($batch_system eq 'PBS') {  
      $batch_script  = "$fntemplate.pbs";
      open(PBS, ">$batch_script") or die("Can not create the PBS batch script");
      print PBS "#!/bin/bash \n";
      print PBS "#PBS -l cput=$time_limit \n";
      print PBS "#PBS -o $fntemplate.pbsout.log \n";
      print PBS "#PBS -e $fntemplate.pbserr.log \n";
      print PBS "source $genie_setup \n"; 
      print PBS "cd $jobs_dir \n";
      print PBS "$gspl2root_cmd \n";
      close(PBS);
     `qsub -q $queue $batch_script`;
   } #PBS
  
   # LSF case
   if($batch_system eq 'LSF') {  
      $batch_script  = "$fntemplate.sh";
      open(LSF, ">$batch_script") or die("Can not create the LSF batch script");
      print LSF "#!/bin/bash \n";
      print LSF "#BSUB-q $queue \n";
      print LSF "#BSUB-c $time_limit \n";
      print LSF "#BSUB-o $fntemplate.lsfout.log \n";
      print LSF "#BSUB-e $fntemplate.lsferr.log \n";
      print LSF "source $genie_setup \n"; 
      print LSF "cd $jobs_dir \n";
      print LSF "$gspl2root_cmd \n";
      close(LSF);
     `bsub < $batch_script`;
   } #LSF
  
}
