#pragma once

#include "spcore_global.h"

#include <QString>
#include <QMap>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariant>
#include <QStringList>
#include <QPair>

#include <map>

class SP_CORE_EXPORT SpaceObjectSet
{
public:
    SpaceObjectSet(const QString& name, const QStringList& enabled);

    inline const QString& getName() const {return this->name;}
    inline void setName(const QString& name) {this->name = name;}
    inline const QStringList& getEnabled() const {return this->list_enabled;}
    inline void setEnabled(const QStringList& enabled) {this->list_enabled = enabled;}

private:
    QString name;
    QStringList list_enabled;
};

class SP_CORE_EXPORT SpaceObject
{

public:

    // TODO: Maybe it would be better to change this, to two fields:
    // cpf provider (all, no tle, custom) and if custom is selected
    // then use custom cpf provider field to get the name
    static constexpr const char* kAllCPFProvider = "All";
    static constexpr const char* kNoTLEProvider = "No TLE";
    // SalaraSpaceObject enums.
    /*!
        \enum SalaraSpaceObject::TrackPolicy
        This enum type specifies the different tracking policies applied to a space object (when it should be followed).
        \value TrackAlways
               The object must be tracked always.
        \value TrackOnlyIfVisible
               The object must be tracked only if is visible (in the twilights for example).
        \value TrackOnlyAtNight
               The object must be tracked only at night.
        \value CustomTrackPolicy1
               Custom policy 1 for extensions.
    */
    enum TrackPolicy
    {
        NO_TRACK_POLICY,
        TRACK_ALWAYS,
        TRACK_ONLY_IF_VISIBLE,
        TRACK_ONLY_AT_NIGHT,
        CUSTOM_TRACK_POLICY_1,
        CUSTOM_TRACK_POLICY_2
    };

    enum EnablementPolicy
    {
        NO_ENABLEMENT_POLICY,
        DISABLED,
        ENABLED,
        ALWAYS_DISABLED,
        ALWAYS_ENABLED
    };

    static const QMap<TrackPolicy, QString> TrackPolicyStringMap;
    static const QMap<EnablementPolicy, QString> EnablementPolicyStringMap;
    static const QMap<QString, SpaceObject::EnablementPolicy> EnablementPolicyEnumMap;

    // SalaraSpaceObject constructor.
    SpaceObject(QString name, QString ILRSname, QString abbreviation,  QString cospar, QString ILRScospar,
                      QString norad, QString sic, QString classification, QString laserid, QString detectorid,
                      QString counterid, QString cpfprovider, QString picture, TrackPolicy trackpolicy,
                      EnablementPolicy enablementpolicy, int npi, int bs, int altitude, int amplification, int priority,
                      double inclination, double rcs, bool llr, bool debris,QMap<QString, QString>& extraparameters):
        name(name), ILRSname(ILRSname), abbreviation(abbreviation), cospar(cospar), ILRSID(ILRScospar),
        norad(norad), sic(sic), classification(classification), laserid(laserid), detectorid(detectorid),
        counterid(counterid), cpfprovider(cpfprovider), picture(picture), trackpolicy(trackpolicy),
        enablementpolicy(enablementpolicy), npi(npi), bs(bs), altitude(altitude), amplification(amplification),
        priority(priority), inclination(inclination), rcs(rcs),  lrr(llr), debris(debris),
        extraparameters(extraparameters), llr_checked(true), debris_checked(true), initialized(true){}

    // SalaraSpaceObject empty constructor.
    SpaceObject():
        name(""), ILRSname(""), abbreviation(""), cospar(""), ILRSID(""), norad(""), sic(""), classification(""),
        laserid(""), detectorid(""), counterid(""), cpfprovider(""), picture(""), trackpolicy(NO_TRACK_POLICY),
        enablementpolicy(NO_ENABLEMENT_POLICY), npi(0), bs(0), altitude(0), amplification(0), priority(-1),
        inclination(-1), rcs(0), lrr(false), debris(false), llr_checked(false), debris_checked(false),
        initialized(false){}

    SpaceObject(const QJsonValue &json_value, const QStringList &extrakeys);

    // SalaraSpaceObject copy constructor.
    // Voy a probar el por defecto.

    // Observadores.
    inline const QString& getName()           const {return this->name;}
    inline const QString& getILRSname()       const {return this->ILRSname;}
    inline const QString& getPreferredName()  const {return this->ILRSname.isEmpty() ? this->name :  this->ILRSname;}
    inline const QString& getAbbreviation()   const {return this->abbreviation;}
    inline const QString& getCospar()         const {return this->cospar;}
    inline const QString& getILRSID()         const {return this->ILRSID;}
    inline const QString& getNorad()          const {return this->norad;}
    inline const QString& getSIC()            const {return this->sic;}
	inline const QString& getClassification() const {return this->classification;}
    inline const QString& getLaserID()        const {return this->laserid;}
    inline const QString& getDetectorID()     const {return this->detectorid;}
    inline const QString& getCounterID()      const {return this->counterid;}
    inline const QString& getCPFProvider()    const {return this->cpfprovider;}
    inline const QString& getPicture()        const {return this->picture;}
    inline TrackPolicy getTrackPolicy()       const {return this->trackpolicy;}
    inline EnablementPolicy getEnablementPolicy() const {return this->enablementpolicy;}
    inline const int& getNormalPointIndicator()   const {return this->npi;}
    inline const int& getBinSize()                const {return this->bs;}
    inline const int& getAltitude()               const {return this->altitude;}
    inline const int& getAmplification()          const {return this->amplification;}
    inline const int& getPriority()               const {return this->priority;}
    inline const double& getInclination()         const {return this->inclination;}
    inline const double& getRadarCrossSection()   const {return this->rcs;}
    inline const bool& hasLRR()  const {return this->lrr;}
    inline const bool& isDebris()                 const {return this->debris;}
    inline const QMap<QString, QString>& getExtraParameters() const {return this->extraparameters;}
    inline const bool& isLaserRetroReflectorChecked()         const {return this->llr_checked;}
    inline const bool& isDebrisChecked()           const {return this->debris_checked;}
    inline const bool& isInitialized()             const {return this->initialized;}

    // SalaraSpaceObject modifiers.
    inline void setName(QString name)                {this->name = name;}
    inline void setILRSname(QString ILRSname)        {this->ILRSname = ILRSname;}
    inline void setAbbreviation(QString abbreviation){this->abbreviation = abbreviation;}
    inline void setCospar(QString cospar)            {this->cospar = cospar;}
    inline void setILRScospar(QString ILRScospar)    {this->ILRSID = ILRScospar;}
    inline void setNorad(QString norad)              {this->norad = norad;}
    inline void setSic(QString sic)                  {this->sic = sic;}
    inline void setClassification(QString classifi)  {this->classification = classifi;}
    inline void setLaserID(QString laserid)          {this->laserid = laserid;}
    inline void setDetectorID(QString detectorid)    {this->detectorid = detectorid;}
    inline void setCounterID(QString counterid)      {this->counterid = counterid;}
    inline void setCPFProvider(QString cpf)          {this->cpfprovider = cpf;}
    inline void setPicture(QString picture)          {this->picture = picture;}
    inline void setTrackPolicy(TrackPolicy policy)          {this->trackpolicy = policy;}
    inline void setEnablementPolicy(EnablementPolicy policy){this->enablementpolicy = policy;}
    inline void setNormalPointIndicator(int npi)     {this->npi = npi;}
    inline void setBinSize(int bs)                   {this->bs = bs;}
    inline void setAltitude(int altitude)            {this->altitude = altitude;}
    inline void setAmplification(int amplification)  {this->amplification = amplification;}
    inline void setPriority(int priority)            {this->priority = priority;}
    inline void setInclination(double inclination)   {this->inclination = inclination;}
    inline void setRadarCrossSection(double rcs)     {this->rcs = rcs;}
    inline void setHasLRR(bool llr) {this->lrr = llr; this->llr_checked = true;}
    inline void setIsDebris(bool debris)             {this->debris = debris; this->debris_checked = true;}
    inline void setExtraParameters(const QMap<QString, QString>& extra){this->extraparameters = extra;}
    inline void setInitialized(bool ini)  {this->initialized = ini;}

    // To string method.
    const QString toString() const;
    // To JSON method.
    const QJsonObject toJson() const;

    // Static method to generate a JsonObject that contents an example empty object.
    static QJsonObject generateEmptyJsonobject();

protected:
    QString name;
    QString ILRSname;
    QString abbreviation;   // In some stations they prefer to work with abbreviations.
    QString cospar;
    QString ILRSID;
    QString norad;
    QString sic;	
    QString classification;	// Type of object in orbit that is (debris, geodetic, rocket body, etc).
    QString laserid;        // Laser ID (for stations with multiple laser systems like ps, ns, etc).
    QString detectorid;     // Detector ID (for stations with multiple photon detector systems like SPAD, etc).
    QString counterid;      // Counter ID (for stations with multiple counter systems (interval, event timer, etc).
    QString cpfprovider;    // Default CPF provider.
    QString picture;        // Name of the satellite picture.
    TrackPolicy trackpolicy;           // Policy to determine when the object must be tracked.
    EnablementPolicy enablementpolicy; // Policy to enable or disable the object.
    int npi;              // Normal Point Indicator.
    int bs;               // Bin Size in Seconds.
    int altitude;         // Altitude in Km.
    int amplification;    // Energy amplification for the laser.
    int priority;         // Tracking priority.
    double inclination;   // Inclination in deg.
    double rcs;           // Radar cross-section (m2).
    bool lrr;             // Laser Retro Reflector (yes/no).
    bool debris;          // Is debris (yes/no).
    QMap<QString, QString> extraparameters; // Extra parameters for customize the object for each station.
    bool llr_checked;
    bool debris_checked;
    bool initialized;
};

Q_DECLARE_METATYPE(SpaceObject*)
Q_DECLARE_METATYPE(SpaceObjectSet*)
