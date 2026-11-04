// Known-answer probe: parses a file with one of the fuzzed OpenMS readers and prints a stable summary.
// test.sh compares the summary with the golden values in mayhem/kat/expected/.
#include <OpenMS/ANALYSIS/TARGETED/TargetedExperiment.h>
#include <OpenMS/CHEMISTRY/AASequence.h>
#include <OpenMS/CHEMISTRY/EmpiricalFormula.h>
#include <OpenMS/DATASTRUCTURES/Param.h>
#include <OpenMS/FORMAT/ConsensusXMLFile.h>
#include <OpenMS/FORMAT/FASTAFile.h>
#include <OpenMS/FORMAT/FeatureXMLFile.h>
#include <OpenMS/FORMAT/IdXMLFile.h>
#include <OpenMS/FORMAT/MascotGenericFile.h>
#include <OpenMS/FORMAT/MzDataFile.h>
#include <OpenMS/FORMAT/MzIdentMLFile.h>
#include <OpenMS/FORMAT/MzMLFile.h>
#include <OpenMS/FORMAT/MzTabFile.h>
#include <OpenMS/FORMAT/MzXMLFile.h>
#include <OpenMS/FORMAT/ParamXMLFile.h>
#include <OpenMS/FORMAT/PepXMLFile.h>
#include <OpenMS/FORMAT/ProtXMLFile.h>
#include <OpenMS/FORMAT/TraMLFile.h>
#include <OpenMS/KERNEL/ConsensusMap.h>
#include <OpenMS/KERNEL/FeatureMap.h>
#include <OpenMS/KERNEL/MSExperiment.h>

#include <cstdio>
#include <cstring>
#include <typeinfo>

using namespace OpenMS;

static void summarize(const PeakMap& m)
{
  printf("KAT: spectra=%zu chromatograms=%zu\n", (size_t)m.size(), (size_t)m.getChromatograms().size());
  if (!m.empty())
  {
    const auto& s = m[0];
    printf("KAT: first: ms_level=%u rt=%.4f peaks=%zu\n", (unsigned)s.getMSLevel(), s.getRT(), (size_t)s.size());
    if (!s.empty()) printf("KAT: first_peak: mz=%.4f intensity=%.2f\n", s[0].getMZ(), (double)s[0].getIntensity());
  }
}

static void summarize(const std::vector<ProteinIdentification>& prots, const PeptideIdentificationList& peps)
{
  printf("KAT: protein_runs=%zu peptide_ids=%zu\n", prots.size(), (size_t)peps.size());
  if (!prots.empty()) printf("KAT: protein_hits=%zu\n", prots[0].getHits().size());
  if (!peps.empty() && !peps[0].getHits().empty())
  {
    printf("KAT: first_hits=%zu first_sequence=%s\n", peps[0].getHits().size(), peps[0].getHits()[0].getSequence().toString().c_str());
  }
}

int main(int argc, char** argv)
{
  if (argc < 3)
  {
    fprintf(stderr, "usage: %s <mode> <file|expr>\n", argv[0]);
    return 2;
  }
  const std::string mode = argv[1];
  const std::string arg = argv[2];
  try
  {
    if (mode == "mzml") { PeakMap m; MzMLFile().load(arg, m); summarize(m); }
    else if (mode == "mzxml") { PeakMap m; MzXMLFile().load(arg, m); summarize(m); }
    else if (mode == "mzdata") { PeakMap m; MzDataFile().load(arg, m); summarize(m); }
    else if (mode == "mgf") { PeakMap m; MascotGenericFile().load(arg, m); summarize(m); }
    else if (mode == "fasta")
    {
      std::vector<FASTAFile::FASTAEntry> e;
      FASTAFile().load(arg, e);
      printf("KAT: entries=%zu\n", e.size());
      if (!e.empty()) printf("KAT: first: id=%s length=%zu\n", e[0].identifier.c_str(), e[0].sequence.size());
    }
    else if (mode == "idxml") { std::vector<ProteinIdentification> pr; PeptideIdentificationList pe; IdXMLFile().load(arg, pr, pe); summarize(pr, pe); }
    else if (mode == "mzid") { std::vector<ProteinIdentification> pr; PeptideIdentificationList pe; MzIdentMLFile().load(arg, pr, pe); summarize(pr, pe); }
    else if (mode == "pepxml") { std::vector<ProteinIdentification> pr; PeptideIdentificationList pe; PepXMLFile().load(arg, pr, pe); summarize(pr, pe); }
    else if (mode == "protxml")
    {
      ProteinIdentification pr; PeptideIdentification pe;
      ProtXMLFile().load(arg, pr, pe);
      printf("KAT: protein_hits=%zu groups=%zu indistinguishable=%zu\n", pr.getHits().size(), pr.getProteinGroups().size(), pr.getIndistinguishableProteins().size());
    }
    else if (mode == "featurexml")
    {
      FeatureMap f; FeatureXMLFile().load(arg, f);
      printf("KAT: features=%zu\n", (size_t)f.size());
      if (!f.empty()) printf("KAT: first: mz=%.4f rt=%.4f intensity=%.2f charge=%d\n", f[0].getMZ(), f[0].getRT(), (double)f[0].getIntensity(), (int)f[0].getCharge());
    }
    else if (mode == "consensusxml")
    {
      ConsensusMap c; ConsensusXMLFile().load(arg, c);
      printf("KAT: consensus=%zu maps=%zu\n", (size_t)c.size(), c.getColumnHeaders().size());
      if (!c.empty()) printf("KAT: first: mz=%.4f rt=%.4f intensity=%.2f elements=%zu\n", c[0].getMZ(), c[0].getRT(), (double)c[0].getIntensity(), (size_t)c[0].size());
    }
    else if (mode == "traml")
    {
      TargetedExperiment t; TraMLFile().load(arg, t);
      printf("KAT: transitions=%zu peptides=%zu proteins=%zu compounds=%zu\n", t.getTransitions().size(), t.getPeptides().size(), t.getProteins().size(), t.getCompounds().size());
      if (!t.getTransitions().empty()) printf("KAT: first: id=%s precursor_mz=%.4f product_mz=%.4f\n", t.getTransitions()[0].getNativeID().c_str(), t.getTransitions()[0].getPrecursorMZ(), t.getTransitions()[0].getProductMZ());
    }
    else if (mode == "mztab")
    {
      MzTab t; MzTabFile().load(arg, t);
      printf("KAT: psm=%zu protein=%zu peptide=%zu small_molecule=%zu\n", t.getPSMSectionRows().size(), t.getProteinSectionRows().size(), t.getPeptideSectionRows().size(), t.getSmallMoleculeSectionRows().size());
    }
    else if (mode == "paramxml")
    {
      Param p; ParamXMLFile().load(arg, p);
      printf("KAT: entries=%zu\n", (size_t)p.size());
      if (p.size() > 0) printf("KAT: first: %s\n", p.begin().getName().c_str());
    }
    else if (mode == "formula") { printf("KAT: monoisotopic=%.4f average=%.4f\n", EmpiricalFormula(arg).getMonoWeight(), EmpiricalFormula(arg).getAverageWeight()); }
    else if (mode == "peptide") { AASequence a = AASequence::fromString(arg); printf("KAT: monoisotopic=%.4f length=%zu\n", a.getMonoWeight(), (size_t)a.size()); }
    else { fprintf(stderr, "unknown mode %s\n", mode.c_str()); return 2; }
  }
  catch (const std::exception& e)
  {
    printf("KAT: error: %s\n", typeid(e).name());
    return 1;
  }
  return 0;
}
