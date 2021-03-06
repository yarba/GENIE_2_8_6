#!/usr/bin/perl

#-----------------------------------------------------------------------------
# Submit jobs for calculating GENIE free-nucleon cross-section splines
# which can then be re-used for calculating nuclear cross-sections
#
# Syntax:
#   shell% perl submit_vN_xsec_calc_jobs.pl <options>
#
# Options:
#    --version       : genie version number
#    --xsplset       : set of splines to generate
#   [--arch]         : <SL4_32bit, SL5_64bit>, default: SL5_64bit
#   [--production]   : default: <version>
#   [--cycle]        : default: 01
#   [--use-valgrind] : default: off
#   [--batch-system] : <PBS, LSF>, default: PBS
#   [--queue]        : default: prod
#   [--softw-topdir] : default: /opt/ppd/t2k/softw/GENIE
#
# Examples:
#   shell% perl submit_vN_xsec_calc_jobs.pl --version v2.6.0 --xsplset chm 
#   shell% perl submit_vN_xsec_calc_jobs.pl --version v2.6.0 --xsplset chm,nue,qel
#   shell% perl submit_vN_xsec_calc_jobs.pl --version v2.6.0 --xsplset all 
#
# Notes:
#   * Use GENIE gspladd utility to merge the job outputs
#
# Tested at the RAL/PPD Tier2 PBS batch farm.
#
# Costas Andreopoulos <costas.andreopoulos \st stfc.ac.uk>
# STFC, Rutherford Appleton Lab
#-----------------------------------------------------------------------------

use File::Path;

# inputs
#  
$iarg=0;
foreach (@ARGV) {
  if($_ eq '--version')       { $genie_version = $ARGV[$iarg+1]; }
  if($_ eq '--xsplset')       { $xsplset       = $ARGV[$iarg+1]; }
  if($_ eq '--arch')          { $arch          = $ARGV[$iarg+1]; }
  if($_ eq '--production')    { $production    = $ARGV[$iarg+1]; }
  if($_ eq '--cycle')         { $cycle         = $ARGV[$iarg+1]; }
  if($_ eq '--use-valgrind')  { $use_valgrind  = $ARGV[$iarg+1]; }
  if($_ eq '--batch-system')  { $batch_system  = $ARGV[$iarg+1]; }
  if($_ eq '--queue')         { $queue         = $ARGV[$iarg+1]; }
  if($_ eq '--softw-topdir')  { $softw_topdir  = $ARGV[$iarg+1]; }           
  $iarg++;
}
die("** Aborting [Undefined set of cross section splines #. Use the --xsplset option]")
unless defined $xsplset;
die("** Aborting [Undefined GENIE version. Use the --version option]")
unless defined $genie_version;

$use_valgrind   = 0                             unless defined $use_valgrind;
$arch           = "SL5_64bit"                   unless defined $arch;
$production     = "$genie_version"              unless defined $production;
$cycle          = "01"                          unless defined $cycle;
$batch_system   = "PBS"                         unless defined $batch_system;
$queue          = "prod"                        unless defined $queue;
$softw_topdir   = "/opt/ppd/t2k/softw/GENIE"    unless defined $softw_topdir;
$genie_setup    = "$softw_topdir/builds/$arch/$genie_version-setup";
$jobs_dir       = "$softw_topdir/scratch/xsec\_vN-$production\_$cycle/";

$nkots = 500;
$emax  = 500;

%NUPDG =   ( 'chm'           => '12,-12,14,-14,16,-16',
             'nue'           => '12,-12,14,-14,16,-16',
             'qel'           => '12,-12,14,-14,16,-16',
             'dis_ebar_cc'   => '-12',
             'dis_ebar_nc'   => '-12',
             'dis_e_cc'      => '12',
             'dis_e_nc'      => '12',
             'dis_mubar_cc'  => '-14',
             'dis_mubar_nc'  => '-14',
             'dis_mu_cc'     => '14',
             'dis_mu_nc'     => '14',
             'dis_taubar_cc' => '-16',
             'dis_taubar_nc' => '-16',
             'dis_tau_cc'    => '16',
             'dis_tau_nc'    => '16',
             'res_ebar_cc'   => '-12',
             'res_ebar_nc'   => '-12',
             'res_e_cc'      => '12',
             'res_e_nc'      => '12',
             'res_mubar_cc'  => '-14',
             'res_mubar_nc'  => '-14',
             'res_mu_cc'     => '14',
             'res_mu_nc'     => '14',
             'res_taubar_cc' => '-16',
             'res_taubar_nc' => '-16',
             'res_tau_cc'    => '16',
             'res_tau_nc'    => '16' );

%TGTPDG =  ( 'chm'           => '1000010010,1000000010',
             'nue'           => '1000010010,1000000010',
             'qel'           => '1000010010,1000000010',
             'dis_ebar_cc'   => '1000010010,1000000010',
             'dis_ebar_nc'   => '1000010010,1000000010',
             'dis_e_cc'      => '1000010010,1000000010',
             'dis_e_nc'      => '1000010010,1000000010',
             'dis_mubar_cc'  => '1000010010,1000000010',
             'dis_mubar_nc'  => '1000010010,1000000010',
             'dis_mu_cc'     => '1000010010,1000000010',
             'dis_mu_nc'     => '1000010010,1000000010',
             'dis_taubar_cc' => '1000010010,1000000010',
             'dis_taubar_nc' => '1000010010,1000000010',
             'dis_tau_cc'    => '1000010010,1000000010',
             'dis_tau_nc'    => '1000010010,1000000010',
             'res_ebar_cc'   => '1000010010,1000000010',
             'res_ebar_nc'   => '1000010010,1000000010',
             'res_e_cc'      => '1000010010,1000000010',
             'res_e_nc'      => '1000010010,1000000010',
             'res_mubar_cc'  => '1000010010,1000000010',
             'res_mubar_nc'  => '1000010010,1000000010',
             'res_mu_cc'     => '1000010010,1000000010',
             'res_mu_nc'     => '1000010010,1000000010',
             'res_taubar_cc' => '1000010010,1000000010',
             'res_taubar_nc' => '1000010010,1000000010',
             'res_tau_cc'    => '1000010010,1000000010',
             'res_tau_nc'    => '1000010010,1000000010' );

%GEVGL =   ( 'chm'           => 'Charm',
             'nue'           => 'NuE',
             'qel'           => 'QE',
             'dis_ebar_cc'   => 'CCDIS',
             'dis_ebar_nc'   => 'NCDIS',
             'dis_e_cc'      => 'CCDIS',
             'dis_e_nc'      => 'NCDIS',
             'dis_mubar_cc'  => 'CCDIS',
             'dis_mubar_nc'  => 'NCDIS',
             'dis_mu_cc'     => 'CCDIS',
             'dis_mu_nc'     => 'NCDIS',
             'dis_taubar_cc' => 'CCDIS',
             'dis_taubar_nc' => 'NCDIS',
             'dis_tau_cc'    => 'CCDIS',
             'dis_tau_nc'    => 'NCDIS',
             'res_ebar_cc'   => 'CCRES',
             'res_ebar_nc'   => 'NCRES',
             'res_e_cc'      => 'CCRES',
             'res_e_nc'      => 'NCRES',
             'res_mubar_cc'  => 'CCRES',
             'res_mubar_nc'  => 'NCRES',
             'res_mu_cc'     => 'CCRES',
             'res_mu_nc'     => 'NCRES',
             'res_taubar_cc' => 'CCRES',
             'res_taubar_nc' => 'NCRES',
             'res_tau_cc'    => 'CCRES',
             'res_tau_nc'    => 'NCRES' );

%OUTXML =  ( 'chm'           => 'pgxspl-chm.xml',
             'nue'           => 'pgxspl-nue.xml',
             'qel'           => 'pgxspl-qel.xml',
             'dis_ebar_cc'   => 'pgxspl-dis_ebar_cc.xml',
             'dis_ebar_nc'   => 'pgxspl-dis_ebar_nc.xml',
             'dis_e_cc'      => 'pgxspl-dis_e_cc.xml',
             'dis_e_nc'      => 'pgxspl-dis_e_nc.xml',
             'dis_mubar_cc'  => 'pgxspl-dis_mubar_cc.xml',
             'dis_mubar_nc'  => 'pgxspl-dis_mubar_nc.xml',
             'dis_mu_cc'     => 'pgxspl-dis_mu_cc.xml',
             'dis_mu_nc'     => 'pgxspl-dis_mu_nc.xml',
             'dis_taubar_cc' => 'pgxspl-dis_taubar_cc.xml',
             'dis_taubar_nc' => 'pgxspl-dis_taubar_nc.xml',
             'dis_tau_cc'    => 'pgxspl-dis_tau_cc.xml',
             'dis_tau_nc'    => 'pgxspl-dis_tau_nc.xml',
             'res_ebar_cc'   => 'pgxspl-res_ebar_cc.xml',
             'res_ebar_nc'   => 'pgxspl-res_ebar_nc.xml',
             'res_e_cc'      => 'pgxspl-res_e_cc.xml',
             'res_e_nc'      => 'pgxspl-res_e_nc.xml',
             'res_mubar_cc'  => 'pgxspl-res_mubar_cc.xml',
             'res_mubar_nc'  => 'pgxspl-res_mubar_nc.xml',
             'res_mu_cc'     => 'pgxspl-res_mu_cc.xml',
             'res_mu_nc'     => 'pgxspl-res_mu_nc.xml',
             'res_taubar_cc' => 'pgxspl-res_taubar_cc.xml',
             'res_taubar_nc' => 'pgxspl-res_taubar_nc.xml',
             'res_tau_cc'    => 'pgxspl-res_tau_cc.xml',
             'res_tau_nc'    => 'pgxspl-res_tau_nc.xml' );

# make the jobs directory
#
mkpath ($jobs_dir, {verbose => 1, mode=>0777});

for my $curr_xsplset (keys %OUTXML)  {
  if($xsplset=~m/$curr_xsplset/ || $xsplset eq "all") {

    #
    # get runnu-dependent info 
    #
    $nu     = $NUPDG   {$curr_xsplset};
    $tgt    = $TGTPDG  {$curr_xsplset};
    $gevgl  = $GEVGL   {$curr_xsplset};
    $outxml = $OUTXML  {$curr_xsplset};

    $fntemplate    = "$jobs_dir/job-$curr_xsplset"; 
    $grep_pipe     = "grep -B 100 -A 30 -i \"warn\\|error\\|fatal\"";
    $valgrind_cmd  = "valgrind --tool=memcheck --error-limit=no --leak-check=yes --show-reachable=yes";
    $gmkspl_opt    = "-p $nu -t $tgt -n $nkots -e $emax -o $outxml --event-generator-list $gevgl";
    $gmkspl_cmd    = "gmkspl $gmkspl_opt | $grep_pipe &> $fntemplate.mkspl.log";

    print "@@ exec: $gmkspl_cmd \n";

    #
    # submit
    #
  
    # PBS case
    if($batch_system eq 'PBS') {
        $batch_script = "$fntemplate.pbs";
        open(PBS, ">$batch_script") or die("Can not create the PBS batch script");
        print PBS "#!/bin/bash \n";
        print PBS "#PBS -o $fntemplate.pbsout.log \n";
        print PBS "#PBS -e $fntemplate.pbserr.log \n";
        print PBS "source $genie_setup \n";
        print PBS "cd $jobs_dir \n";
        print PBS "$gmkspl_cmd \n";
        close(PBS);
        `qsub -q $queue $batch_script`;
    } #PBS

    # LSF case
    if($batch_system eq 'LSF') {
        $batch_script = "$fntemplate.sh";
        open(LSF, ">$batch_script") or die("Can not create the LSF batch script");
        print LSF "#!/bin/bash \n";
        print LSF "#BSUB-q $queue \n";
        print LSF "#BSUB-o $fntemplate.lsfout.log \n";
        print LSF "#BSUB-e $fntemplate.lsferr.log \n";
        print LSF "source $genie_setup \n";
        print LSF "cd $jobs_dir \n";
        print LSF "$gmkspl_cmd \n";
        close(LSF);
        `bsub < $batch_script`;
    } #LSF

  }
}
