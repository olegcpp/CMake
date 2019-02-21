/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGenInitializer_h
#define cmQtAutoGenInitializer_h

#include "cmConfigure.h" // IWYU pragma: keep
#include "cmGeneratedFileStream.h"
#include "cmQtAutoGen.h"

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

class cmGeneratorTarget;
class cmTarget;
class cmQtAutoGenGlobalInitializer;

/// @brief Initializes the QtAutoGen generators
class cmQtAutoGenInitializer : public cmQtAutoGen
{
public:
  /// @brief Rcc job information
  class Qrc
  {
  public:
    std::string LockFile;
    std::string QrcFile;
    std::string QrcName;
    std::string PathChecksum;
    std::string InfoFile;
    std::string SettingsFile;
    std::map<std::string, std::string> ConfigSettingsFile;
    std::string RccFile;
    bool Generated = false;
    bool Unique = false;
    std::vector<std::string> Options;
    std::vector<std::string> Resources;
  };

  /// @brief Abstract moc/uic/rcc generator variables base class
  struct GenVarsT
  {
    bool Enabled = false;
    // Generator type/name
    GenT Gen;
    std::string const& GenName;
    std::string const& GenNameUpper;
    // Executable
    std::string ExecutableTargetName;
    cmGeneratorTarget* ExecutableTarget = nullptr;
    std::string Executable;

    /// @brief Constructor
    GenVarsT(GenT gen, std::string const& genName,
             std::string const& genNameUpper)
      : Gen(gen)
      , GenName(genName)
      , GenNameUpper(genNameUpper){};
  };

  /// @brief Writes a CMake info file
  class InfoWriter
  {
  public:
    /// @brief Open the given file
    InfoWriter(std::string const& filename);

    /// @return True if the file is open
    explicit operator bool() const { return static_cast<bool>(Ofs_); }

    void Write(const char* text) { Ofs_ << text; }
    void Write(const char* key, std::string const& value);
    void WriteUInt(const char* key, unsigned int value);

    template <class C>
    void WriteStrings(const char* key, C const& container);
    void WriteConfig(const char* key,
                     std::map<std::string, std::string> const& map);
    template <class C>
    void WriteConfigStrings(const char* key,
                            std::map<std::string, C> const& map);
    void WriteNestedLists(const char* key,
                          std::vector<std::vector<std::string>> const& lists);

  private:
    template <class IT>
    static std::string ListJoin(IT it_begin, IT it_end);
    static std::string ConfigKey(const char* key, std::string const& config);

  private:
    cmGeneratedFileStream Ofs_;
  };

public:
  /// @return The detected Qt version and the required Qt major version
  static std::pair<IntegerVersion, unsigned int> GetQtVersion(
    cmGeneratorTarget const* target);

  cmQtAutoGenInitializer(cmQtAutoGenGlobalInitializer* globalInitializer,
                         cmGeneratorTarget* target,
                         IntegerVersion const& qtVersion, bool mocEnabled,
                         bool uicEnabled, bool rccEnabled,
                         bool globalAutogenTarget, bool globalAutoRccTarget);

  bool InitCustomTargets();
  bool SetupCustomTargets();

private:
  bool InitMoc();
  bool InitUic();
  bool InitRcc();

  bool InitScanFiles();
  bool InitAutogenTarget();
  bool InitRccTargets();

  bool SetupWriteAutogenInfo();
  bool SetupWriteRccInfo();

  void RegisterGeneratedSource(std::string const& filename);
  bool AddGeneratedSource(std::string const& filename, GenVarsT const& genVars,
                          bool prepend = false);
  bool AddToSourceGroup(std::string const& fileName,
                        std::string const& genNameUpper);

  bool GetQtExecutable(GenVarsT& genVars, const std::string& executable,
                       bool ignoreMissingTarget, std::string* output) const;

  bool RccListInputs(std::string const& fileName,
                     std::vector<std::string>& files,
                     std::string& errorMessage);

private:
  cmQtAutoGenGlobalInitializer* GlobalInitializer;
  cmGeneratorTarget* Target;

  // Configuration
  IntegerVersion QtVersion;
  bool MultiConfig = false;
  std::string ConfigDefault;
  std::vector<std::string> ConfigsList;
  std::string Verbosity;
  std::string TargetsFolder;

  /// @brief Common directories
  struct
  {
    std::string Info;
    std::string Build;
    std::string Work;
    std::string Include;
    std::map<std::string, std::string> ConfigInclude;
  } Dir;

  /// @brief Autogen target variables
  struct
  {
    std::string Name;
    bool GlobalTarget = false;
    // Settings
    std::string Parallel;
    // Configuration files
    std::string InfoFile;
    std::string SettingsFile;
    std::map<std::string, std::string> ConfigSettingsFile;
    // Dependencies
    bool DependOrigin = false;
    std::set<std::string> DependFiles;
    std::set<cmTarget*> DependTargets;
    // Sources to process
    std::vector<std::string> Headers;
    std::vector<std::string> Sources;
    std::vector<std::string> HeadersGenerated;
    std::vector<std::string> SourcesGenerated;
  } AutogenTarget;

  /// @brief Moc only variables
  struct MocT : public GenVarsT
  {
    std::string PredefsCmd;
    std::set<std::string> Skip;
    std::vector<std::string> Includes;
    std::map<std::string, std::vector<std::string>> ConfigIncludes;
    std::set<std::string> Defines;
    std::map<std::string, std::set<std::string>> ConfigDefines;
    std::string MocsCompilation;

    /// @brief Constructor
    MocT()
      : GenVarsT(cmQtAutoGen::GenT::MOC, cmQtAutoGen::GenNameMoc,
                 cmQtAutoGen::GenNameMocUpper){};
  } Moc;

  /// @brief Uic only variables
  struct UicT : public GenVarsT
  {
    std::set<std::string> Skip;
    std::vector<std::string> SearchPaths;
    std::vector<std::string> Options;
    std::map<std::string, std::vector<std::string>> ConfigOptions;
    std::vector<std::string> FileFiles;
    std::vector<std::vector<std::string>> FileOptions;

    /// @brief Constructor
    UicT()
      : GenVarsT(cmQtAutoGen::GenT::UIC, cmQtAutoGen::GenNameUic,
                 cmQtAutoGen::GenNameUicUpper){};
  } Uic;

  /// @brief Rcc only variables
  struct RccT : public GenVarsT
  {
    bool GlobalTarget = false;
    std::vector<std::string> ListOptions;
    std::vector<Qrc> Qrcs;

    /// @brief Constructor
    RccT()
      : GenVarsT(cmQtAutoGen::GenT::RCC, cmQtAutoGen::GenNameRcc,
                 cmQtAutoGen::GenNameRccUpper){};
  } Rcc;
};

#endif
