#include "linuxffbconditioneffect.h"
#include "globalsettings.h"

LinuxFFBConditionEffect::LinuxFFBConditionEffect() :
  LinuxFFBEffect(FFBEffectTypes::CONDITION)
{}

struct ff_effect* LinuxFFBConditionEffect::createFFStruct()
{
  struct ff_effect* eff = LinuxFFBEffect::createFFStruct(m_params);

  eff->u.condition[0].center = m_params->center[FFBConditionEffectParameters::Axis::X];
  eff->u.condition[0].deadband = m_params->deadband[FFBConditionEffectParameters::Axis::X];
  eff->u.condition[0].left_coeff = m_params->leftCoeff[FFBConditionEffectParameters::Axis::X];
  eff->u.condition[0].right_coeff = m_params->rightCoeff[FFBConditionEffectParameters::Axis::X];
  eff->u.condition[0].left_saturation = m_params->leftSat[FFBConditionEffectParameters::Axis::X];
  eff->u.condition[0].right_saturation = m_params->rightSat[FFBConditionEffectParameters::Axis::X];

  eff->u.condition[1].center = m_params->center[FFBConditionEffectParameters::Axis::Y];
  eff->u.condition[1].deadband = m_params->deadband[FFBConditionEffectParameters::Axis::Y];
  eff->u.condition[1].left_coeff = m_params->leftCoeff[FFBConditionEffectParameters::Axis::Y];
  eff->u.condition[1].right_coeff = m_params->rightCoeff[FFBConditionEffectParameters::Axis::Y];
  eff->u.condition[1].left_saturation = m_params->leftSat[FFBConditionEffectParameters::Axis::Y];
  eff->u.condition[1].right_saturation = m_params->rightSat[FFBConditionEffectParameters::Axis::Y];

  switch (m_params->subtype) {
    case ConditionSubtypes::DAMPER:
      eff->type = FF_DAMPER;
      break;
    case ConditionSubtypes::FRICTION:
      eff->type = FF_FRICTION;
      break;
    case ConditionSubtypes::INERTIA:
      eff->type = FF_INERTIA;
      break;
    case ConditionSubtypes::SPRING:
      eff->type = FF_SPRING;
      break;
    default:
      qCritical("Unknown subtype");
      delete eff;
      return nullptr;
  }

  return eff;
}

bool LinuxFFBConditionEffect::setParameters(const std::shared_ptr<FFBEffectParameters> params)
{
  try {
    return setParameters(std::dynamic_pointer_cast<FFBConditionEffectParameters>(params));
  } catch (const std::bad_cast& ex) {
    qCritical(ex.what());
    return false;
  }
  return false;
}

bool LinuxFFBConditionEffect::setParameters(const std::shared_ptr<FFBConditionEffectParameters> params)
{
  if (!GlobalSettings::GS()->doSanityChecks)
    return true;

  if (!checkGenericParameters(params))
    return false;

  if (!checkBoundsInclusive(params->center[FFBConditionEffectParameters::Axis::X], -0x7FFF, 0x7FFF)) {
    reportError("Center X must be within <-32767; 32767>");
    return false;
  }
  if (!checkBoundsInclusive(params->center[FFBConditionEffectParameters::Axis::Y], -0x7FFF, 0x7FFF)) {
    reportError("Center Y must be within <-32767; 32767>");
    return false;
  }

  if (!checkBoundsInclusive(params->deadband[FFBConditionEffectParameters::Axis::X], 0, 0xFFFF)) {
    reportError("Deadband X must be within <0; 65535>");
    return false;
  }
  if (!checkBoundsInclusive(params->deadband[FFBConditionEffectParameters::Axis::Y], 0, 0xFFFF)) {
    reportError("Deadband Ymust be within <0; 65535>");
    return false;
  }

  if (!checkBoundsInclusive(params->leftCoeff[FFBConditionEffectParameters::Axis::X], -0x7FFF, 0x7FFF)) {
    reportError("Left coefficient X must be within <-32767; 32767>");
    return false;
  }
  if (!checkBoundsInclusive(params->leftCoeff[FFBConditionEffectParameters::Axis::Y], -0x7FFF, 0x7FFF)) {
    reportError("Left coefficient Y must be within <-32767; 32767>");
    return false;
  }

  if (!checkBoundsInclusive(params->rightCoeff[FFBConditionEffectParameters::Axis::X], -0x7FFF, 0x7FFF)) {
    reportError("Right coefficient X must be within <-32767; 32767>");
    return false;
  }
  if (!checkBoundsInclusive(params->rightCoeff[FFBConditionEffectParameters::Axis::Y], -0x7FFF, 0x7FFF)) {
    reportError("Right coefficient Y must be within <-32767; 32767>");
    return false;
  }

  if (!checkBoundsInclusive(params->leftSat[FFBConditionEffectParameters::Axis::X], 0, 0xFFFF)) {
    reportError("Left saturation X must be within <0; 65535>");
    return false;
  }
  if (!checkBoundsInclusive(params->leftSat[FFBConditionEffectParameters::Axis::Y], 0, 0xFFFF)) {
    reportError("Left saturation Y must be within <0; 65535>");
    return false;
  }

  if (!checkBoundsInclusive(params->rightSat[FFBConditionEffectParameters::Axis::X], 0, 0xFFFF)) {
    reportError("Right saturation X must be within <0; 65535>");
    return false;
  }
  if (!checkBoundsInclusive(params->rightSat[FFBConditionEffectParameters::Axis::Y], 0, 0xFFFF)) {
    reportError("Right saturation Y must be within <0; 65535>");
    return false;
  }

  if (params->subtype == ConditionSubtypes::NONE) {
    reportError("Invalid subtype");
    return false;
  }

  m_params = params;
  return true;
}

bool LinuxFFBConditionEffect::operator==(const FFBEffect& other) const
{
  if (this->type() != other.type())
    return false;
  else {
    try {
      const LinuxFFBConditionEffect& cother = dynamic_cast<const LinuxFFBConditionEffect&>(other);
      return this->m_params->subtype == cother.m_params->subtype;
    } catch (std::bad_cast&) {
      return false;
    }
  }
}
