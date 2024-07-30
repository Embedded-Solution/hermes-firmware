#include "WaterTouchSensor.h"

WaterTouchSensor::WaterTouchSensor(uint8_t pin, uint16_t threshold) 
    : _pin(pin), _threshold(threshold), _airIndex(0), _waterIndex(0) {
    for (uint8_t i = 0; i < SAMPLE_COUNT; ++i) {
        _airValues[i] = 0xFFFF;  // Utiliser une valeur invalide par défaut
        _waterValues[i] = 0xFFFF;  // Utiliser une valeur invalide par défaut
    }
}

void WaterTouchSensor::begin() {
    _preferences.begin("waterSensor", false);
    loadFromMemory();
    updateTouchInterrupt();
}

bool WaterTouchSensor::isWaterDetected() {
    uint16_t value = touchRead(_pin);
    return value > _threshold;
}

void WaterTouchSensor::setThreshold(uint16_t threshold) {
    _threshold = threshold;
    saveToMemory();
    updateTouchInterrupt();
}

uint16_t WaterTouchSensor::getThreshold() const {
    return _threshold;
}

void WaterTouchSensor::updateAirValue(uint16_t value) {
    _airValues[_airIndex] = value;
    _airIndex = (_airIndex + 1) % SAMPLE_COUNT;
    saveToMemory();
}

void WaterTouchSensor::updateWaterValue(uint16_t value) {
    _waterValues[_waterIndex] = value;
    _waterIndex = (_waterIndex + 1) % SAMPLE_COUNT;
    saveToMemory();
}

float WaterTouchSensor::getLowLimit() const {
    return calculateAverage(_airValues, SAMPLE_COUNT);
}

float WaterTouchSensor::getHighLimit() const {
    return calculateAverage(_waterValues, SAMPLE_COUNT);
}

void WaterTouchSensor::updateThreshold() {
    float lowLimit = getLowLimit();
    float highLimit = getHighLimit();
    if (lowLimit != -1 && highLimit != -1) {
        _threshold = static_cast<uint16_t>((lowLimit + highLimit) / 2);
        saveToMemory();
        updateTouchInterrupt();
    }
}

float WaterTouchSensor::calculateAverage(const uint16_t values[], uint8_t count) const {
    uint32_t sum = 0;
    uint8_t validCount = 0;
    for (uint8_t i = 0; i < count; ++i) {
        if (isValueValid(values[i])) {
            sum += values[i];
            validCount++;
        }
    }
    if (validCount == 0) {
        return -1;  // Aucun élément valide
    }
    return static_cast<float>(sum) / validCount;
}

bool WaterTouchSensor::isValueValid(uint16_t value) const {
    return value != 0xFFFF;  // Vérifier si la valeur est valide
}

void WaterTouchSensor::loadFromMemory() {
    _threshold = _preferences.getUInt("threshold", _threshold);
    for (uint8_t i = 0; i < SAMPLE_COUNT; ++i) {
        _airValues[i] = _preferences.getUInt(("airValue" + String(i)).c_str(), 0xFFFF);
        _waterValues[i] = _preferences.getUInt(("waterValue" + String(i)).c_str(), 0xFFFF);
    }
    updateThreshold();  // Mettre à jour le seuil après le chargement des valeurs
}

void WaterTouchSensor::saveToMemory() {
    _preferences.putUInt("threshold", _threshold);
    for (uint8_t i = 0; i < SAMPLE_COUNT; ++i) {
        _preferences.putUInt(("airValue" + String(i)).c_str(), _airValues[i]);
        _preferences.putUInt(("waterValue" + String(i)).c_str(), _waterValues[i]);
    }
}

void WaterTouchSensor::updateTouchInterrupt() {
    touchAttachInterrupt(_pin, nullptr, _threshold);
}
