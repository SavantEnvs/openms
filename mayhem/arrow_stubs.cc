// Apache Arrow/Parquet is not available in the fuzz image, so the Parquet readers/writers that
// FileHandler references are stubbed out as "unsupported". None of the fuzzed parsers reach them.
#include <OpenMS/FORMAT/ConsensusMapArrowIO.h>
#include <OpenMS/FORMAT/DATAACCESS/MSChromatogramParquetConsumer.h>
#include <OpenMS/FORMAT/FeatureMapArrowIO.h>
#include <OpenMS/FORMAT/PSMArrowIO.h>

namespace OpenMS
{
  bool FeatureMapArrowIO::exportToParquet(const FeatureMap&, const std::string&, const ParquetWriteConfig&) { return false; }
  bool FeatureMapArrowIO::importFromParquet(const std::string&, FeatureMap&) { return false; }
  bool ConsensusMapArrowIO::exportToParquet(const ConsensusMap&, const std::string&, const ParquetWriteConfig&) { return false; }
  bool ConsensusMapArrowIO::importFromParquet(const std::string&, ConsensusMap&) { return false; }
  bool PSMArrowIO::exportToParquet(const std::vector<ProteinIdentification>&, const PeptideIdentificationList&, const std::string&, bool, const ParquetWriteConfig&) { return false; }
  bool PSMArrowIO::importFromParquet(const std::string&, std::vector<ProteinIdentification>&, PeptideIdentificationList&) { return false; }

  class MSChromatogramParquetConsumerImpl {};
  MSChromatogramParquetConsumer::MSChromatogramParquetConsumer(const std::string&, UInt64, const std::string&, const OpenSwath::LightTargetedExperiment&) {}
  MSChromatogramParquetConsumer::~MSChromatogramParquetConsumer() = default;
  void MSChromatogramParquetConsumer::consumeSpectrum(SpectrumType&) {}
  void MSChromatogramParquetConsumer::consumeChromatogram(ChromatogramType&) {}
  void MSChromatogramParquetConsumer::finalize() {}
  void MSChromatogramParquetConsumer::setExpectedSize(Size, Size) {}
  void MSChromatogramParquetConsumer::setExperimentalSettings(const ExperimentalSettings&) {}
}
