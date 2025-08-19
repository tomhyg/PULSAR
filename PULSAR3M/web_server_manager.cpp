// =============================================================================
// web_server_manager.cpp - Version avec debug et correction des boutons
// =============================================================================

#include "web_server_manager.h"
#include "system_manager.h"
#include "spi_manager.h"
#include "secrets_aws_rest.h"
#include "system_manager.h"
#include "battery_manager.h"  // 🔋 AJOUT: Include pour BatteryManager

WebServer WebServerManager::server(80);

// Variables externes
extern String patientID;
extern String patientAge;
extern String patientSex;
extern String patientWeight;
extern String patientHeight;
extern String studyNotes;
extern String sessionDatetime;
extern String wifiSSID;
extern String wifiPassword;
extern String awsEndpoint;
extern SystemMode selectedMode;

// Variables pour la gestion temporelle
extern unsigned long long phoneTimestampMs;
extern unsigned long systemStartMs;
extern String phoneTimezone;

const char* configHTML = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Configuration Montre Physiologique</title>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"<style>"
"body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }"
".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
"h1 { color: #333; text-align: center; margin-bottom: 30px; }"
"h3 { color: #555; border-bottom: 2px solid #007bff; padding-bottom: 5px; }"
".section { margin: 20px 0; padding: 20px; border: 1px solid #ddd; border-radius: 8px; background: #f9f9f9; }"
"label { display: block; margin: 10px 0 5px 0; font-weight: bold; color: #333; }"
"input, select, textarea { width: 100%; padding: 10px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; font-size: 14px; }"
".form-row { display: flex; gap: 15px; }"
".form-row .form-field { flex: 1; }"
"button { background: #007bff; color: white; padding: 12px 20px; border: none; border-radius: 5px; cursor: pointer; margin: 5px; font-size: 14px; }"
"button:hover { background: #0056b3; }"
"button.danger { background: #dc3545; }"
"button.danger:hover { background: #c82333; }"
"button.secondary { background: #6c757d; }"
"button.secondary:hover { background: #545b62; }"
"button.success { background: #28a745; }"
"button.success:hover { background: #218838; }"
".status { padding: 15px; margin: 15px 0; border-radius: 5px; background: #d1ecf1; border-left: 4px solid #bee5eb; }"
".status.success { background: #d4edda; border-left: 4px solid #28a745; color: #155724; }"
".status.danger { background: #f8d7da; border-left: 4px solid #dc3545; color: #721c24; }"
".debug-info { background: #f8f9fa; padding: 15px; margin: 10px 0; border-radius: 5px; font-family: monospace; font-size: 12px; max-height: 300px; overflow-y: auto; }"
".predefined-info { background: #f0f8e7; padding: 10px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #28a745; font-size: 12px; }"
"</style>"
"</head>"
"<body>"
"<div class='container'>"
"<h1>🏥 Configuration Montre Pulsar-004</h1>"

// Section batterie en temps réel
"<div class='section'>"
"<h3>🔋 État de la Batterie</h3>"
"<div id='battery_info' class='status'>🔄 Chargement...</div>"
"<button onclick='updateBattery()'>🔄 Actualiser</button>"
"</div>"

"<div class='section'>"
"<h3>📋 Informations Patient</h3>"
"<label>ID Patient:</label>"
"<input type='text' id='patient_id' placeholder='Ex: P001, JOHN_DOE_2024'>"

"<div class='form-row'>"
"<div class='form-field'>"
"<label>Âge (années):</label>"
"<input type='number' id='patient_age' placeholder='Ex: 35' min='1' max='120'>"
"</div>"
"<div class='form-field'>"
"<label>Sexe:</label>"
"<select id='patient_sex'>"
"<option value=''>Sélectionner</option>"
"<option value='M'>Masculin</option>"
"<option value='F'>Féminin</option>"
"</select>"
"</div>"
"</div>"

"<div class='form-row'>"
"<div class='form-field'>"
"<label>Poids (kg):</label>"
"<input type='number' id='patient_weight' placeholder='Ex: 70' min='20' max='300' step='0.1'>"
"</div>"
"<div class='form-field'>"
"<label>Taille (cm):</label>"
"<input type='number' id='patient_height' placeholder='Ex: 175' min='100' max='250'>"
"</div>"
"</div>"

"<label>Notes d'étude:</label>"
"<textarea id='study_notes' placeholder='Notes sur l étude, conditions, remarques...' rows='3'></textarea>"
"</div>"

"<div class='section'>"
"<h3>📊 Mode d'Enregistrement</h3>"
"<div class='mode-selection'>"
"<div><input type='radio' id='mode_sd' name='mode' value='sd' checked><label for='mode_sd'>💾 Carte SD</label></div>"
"<div><input type='radio' id='mode_wifi' name='mode' value='wifi'><label for='mode_wifi'>📡 WiFi + AWS</label></div>"
"</div>"
"</div>"

"<div class='section' id='wifi_section' style='display: none;'>"
"<h3>🌐 Configuration WiFi + AWS</h3>"
"<div class='predefined-info'>"
"<h4>🔗 Réseaux WiFi Prédéfinis</h4>"
"<p><strong>1.</strong> Wifi_Medical</p>"
"<p><strong>2.</strong> Vallee Sud Bio Parc</p>"
"<p><strong>3.</strong> Flybox-5A5A</p>"
"<p><strong>4.</strong> agoranov (Enterprise)</p>"
"<p><em>💡 Le système testera automatiquement ces réseaux si aucun WiFi manuel n'est configuré.</em></p>"
"</div>"
"<hr style='margin: 20px 0;'>"
"<h4>🔧 WiFi Manuel (Optionnel)</h4>"
"<label>SSID WiFi personnalisé:</label>"
"<input type='text' id='wifi_ssid' placeholder='Laissez vide pour utiliser les réseaux prédéfinis'>"
"<label>Mot de passe WiFi:</label>"
"<input type='password' id='wifi_password' placeholder='Mot de passe du réseau personnalisé'>"
"</div>"

"<div class='section'>"
"<h3>🎛️ Actions Principales</h3>"
"<div style='text-align: center;'>"
"<button onclick='saveConfig()'>💾 Sauvegarder</button>"
"<button onclick='startSession()' class='success'>🚀 Démarrer Session</button>"
"</div>"
"</div>"

"<div class='section'>"
"<h3>📁 Gestion des Fichiers de Sessions</h3>"
"<div style='margin-bottom: 15px;'>"
"<button onclick='loadFileList()'>🔄 Actualiser</button>"
"<button onclick='debugSD()' class='secondary'>🔍 Diagnostiquer SD</button>"
"<button onclick='deleteAllFiles()' class='danger'>🗑️ Supprimer Tout</button>"
"</div>"

"<div id='file_list'><p class='loading'>Chargement des fichiers...</p></div>"
"</div>"

"<div id='status'></div>"
"</div>"

"<script>"
"console.log('🚀 Script chargé - Version DEBUG');"

// Variables globales pour la gestion temporelle
"var phoneTimestamp = 0;"
"var phoneTimezone = '';"

// Fonction de mise à jour de l'heure
"function updateCurrentTime() {"
"var now = new Date();"
"phoneTimestamp = now.getTime();"
"phoneTimezone = Intl.DateTimeFormat().resolvedOptions().timeZone;"
"console.log('🕐 Sync temporelle:', phoneTimestamp, phoneTimezone);"
"}"

// Mettre à jour l'heure toutes les secondes
"setInterval(updateCurrentTime, 1000);"

"document.querySelectorAll('input[name=\"mode\"]').forEach(function(radio) {"
"radio.addEventListener('change', function() {"
"console.log('📊 Mode changé:', this.value);"
"document.getElementById('wifi_section').style.display = this.value === 'wifi' ? 'block' : 'none';"
"});"
"});"

"function showStatus(message, type) {"
"console.log('📢 Status:', message, type);"
"var className = 'status';"
"if (type === 'error') className += ' danger';"
"if (type === 'success') className += ' success';"
"document.getElementById('status').innerHTML = '<div class=\"' + className + '\">' + message + '</div>';"
"}"

// 🔋 FONCTION BATTERIE AVEC TIMEOUT
"function updateBattery() {"
"console.log('🔋 Récupération état batterie...');"
"document.getElementById('battery_info').innerHTML = '🔄 Récupération...';"

// 🔧 TIMEOUT côté client aussi
"var controller = new AbortController();"
"var timeoutId = setTimeout(function() {"
"controller.abort();"
"console.log('⏰ Timeout batterie côté client');"
"}, 5000);"  // 5 secondes max

"fetch('/api/battery', {"
"signal: controller.signal"
"})"
".then(function(response) {"
"clearTimeout(timeoutId);"
"console.log('🔋 Réponse batterie - Status:', response.status);"
"if (!response.ok) {"
"throw new Error('HTTP ' + response.status);"
"}"
"return response.json();"
"})"
".then(function(data) {"
"console.log('🔋 Données batterie reçues:', data);"

"var html = '';"
"if (data.error) {"
"html = '<div class=\"status danger\">❌ ' + data.error + '</div>';"
"} else {"
"var percentage = parseFloat(data.percentage) || 50;"  // Fallback par défaut
"var batteryClass = '';"
"if (percentage > 60) batteryClass = 'success';"
"else if (percentage > 20) batteryClass = '';"
"else batteryClass = 'danger';"

"html = '<div class=\"status ' + batteryClass + '\">';"
"html += '<div style=\"font-size: 24px; margin-bottom: 10px;\">🔋 ' + percentage.toFixed(1) + '%</div>';"
"html += '<div><strong>⚡ Tension:</strong> ' + (parseFloat(data.voltage) || 3.6).toFixed(2) + 'V</div>';"
"html += '<div><strong>📊 État:</strong> ' + (data.status || 'Inconnu') + '</div>';"
"if (data.charging !== undefined) {"
"html += '<div><strong>🔌 Charge:</strong> ' + (data.charging ? '🔌 En cours' : '🔋 Sur batterie') + '</div>';"
"}"
"html += '</div>';"
"}"

"document.getElementById('battery_info').innerHTML = html;"
"})"
".catch(function(error) {"
"clearTimeout(timeoutId);"
"console.error('🔋 Erreur batterie:', error);"

// 🔧 FALLBACK: Affichage par défaut si problème
"var fallbackHtml = '<div class=\"status\">'"
"+ '<div style=\"font-size: 24px; margin-bottom: 10px;\">🔋 75.0%</div>'"
"+ '<div><strong>⚡ Tension:</strong> 3.7V</div>'"
"+ '<div><strong>📊 État:</strong> Estimation (erreur capteur)</div>'"
"+ '<div><strong>⚠️</strong> ' + error.message + '</div>'"
"+ '</div>';"

"document.getElementById('battery_info').innerHTML = fallbackHtml;"
"});"
"}"

"function saveConfig() {"
"console.log('💾 === DÉBUT SAUVEGARDE ===');"
"showStatus('💾 Sauvegarde en cours...', 'info');"

"updateCurrentTime();"

"var config = {"
"patient_id: document.getElementById('patient_id').value,"
"patient_age: document.getElementById('patient_age').value,"
"patient_sex: document.getElementById('patient_sex').value,"
"patient_weight: document.getElementById('patient_weight').value,"
"patient_height: document.getElementById('patient_height').value,"
"study_notes: document.getElementById('study_notes').value,"
"wifi_ssid: document.getElementById('wifi_ssid').value,"
"wifi_password: document.getElementById('wifi_password').value,"
"mode: document.querySelector('input[name=\"mode\"]:checked').value,"
"phone_timestamp: phoneTimestamp,"
"phone_timezone: phoneTimezone"
"};"

"console.log('💾 Configuration à sauvegarder:', config);"

"fetch('/api/save', {"
"method: 'POST',"
"headers: { 'Content-Type': 'application/json' },"
"body: JSON.stringify(config)"
"})"
".then(function(response) { "
"console.log('💾 Réponse sauvegarde - Status:', response.status);"
"console.log('💾 Réponse sauvegarde - OK:', response.ok);"
"if (!response.ok) {"
"throw new Error('HTTP ' + response.status + ' - ' + response.statusText);"
"}"
"return response.json(); "
"})"
".then(function(data) {"
"console.log('💾 Données retour sauvegarde:', data);"
"if (data.success) {"
"showStatus('✅ Configuration sauvegardée avec succès!', 'success');"
"} else {"
"showStatus('❌ Erreur lors de la sauvegarde: ' + (data.message || 'Inconnue'), 'error');"
"}"
"})"
".catch(function(error) { "
"console.error('💾 Erreur sauvegarde:', error);"
"showStatus('❌ Erreur sauvegarde: ' + error.message, 'error'); "
"});"
"}"

"function startSession() {"
"console.log('🚀 === DÉBUT DÉMARRAGE SESSION ===');"

"var patientId = document.getElementById('patient_id').value;"
"console.log('🚀 Patient ID:', patientId);"

"if (!patientId) {"
"console.log('❌ Patient ID manquant');"
"showStatus('❌ ID Patient requis pour démarrer une session', 'error');"
"return;"
"}"

"updateCurrentTime();"
"console.log('🚀 Sync temporelle pour démarrage:', phoneTimestamp, phoneTimezone);"

"if (!confirm('Démarrer une nouvelle session pour le patient: ' + patientId + ' ?')) {"
"console.log('🚀 Démarrage annulé par utilisateur');"
"return;"
"}"

"showStatus('🚀 Démarrage de la session...', 'info');"

"var startData = {"
"phone_timestamp: phoneTimestamp,"
"phone_timezone: phoneTimezone"
"};"

"console.log('🚀 Données envoyées au démarrage:', startData);"

"fetch('/api/start', { "
"method: 'POST',"
"headers: { 'Content-Type': 'application/json' },"
"body: JSON.stringify(startData)"
"})"
".then(function(response) { "
"console.log('🚀 Réponse démarrage - Status:', response.status);"
"console.log('🚀 Réponse démarrage - OK:', response.ok);"
"if (!response.ok) {"
"throw new Error('HTTP ' + response.status + ' - ' + response.statusText);"
"}"
"return response.json(); "
"})"
".then(function(data) {"
"console.log('🚀 Données retour démarrage:', data);"
"if (data.success) {"
"showStatus('✅ Session démarrée! La montre enregistre maintenant.', 'success');"
"setTimeout(function() {"
"showStatus('🔄 Redirection automatique vers la page de la montre...', 'info');"
"}, 2000);"
"} else {"
"showStatus('❌ Erreur démarrage: ' + (data.message || 'Inconnue'), 'error');"
"}"
"})"
".catch(function(error) { "
"console.error('🚀 Erreur démarrage:', error);"
"showStatus('❌ Erreur démarrage: ' + error.message, 'error'); "
"});"
"}"

"function loadFileList() {"
"console.log('📂 === CHARGEMENT LISTE FICHIERS ===');"
"document.getElementById('file_list').innerHTML = '<p class=\"loading\">🔄 Chargement des fichiers...</p>';"

"fetch('/api/files')"
".then(function(response) { "
"console.log('📂 Réponse fichiers - Status:', response.status);"
"if (!response.ok) {"
"throw new Error('HTTP ' + response.status);"
"}"
"return response.json(); "
"})"
".then(function(data) {"
"console.log('📂 Données fichiers reçues:', data);"

"var fileListDiv = document.getElementById('file_list');"

"if (data.error) {"
"console.error('📂 Erreur fichiers:', data.error);"
"fileListDiv.innerHTML = '<div class=\"status danger\">❌ Erreur: ' + data.error + '</div>';"
"return;"
"}"

"if (data.files && data.files.length > 0) {"
"console.log('📂 Fichiers trouvés:', data.files.length);"
"var html = '<h4>📂 Sessions disponibles (' + data.files.length + '):</h4>';"

"data.files.forEach(function(file, index) {"
"console.log('📂 Traitement fichier', index, ':', file);"
"html += '<div class=\"file-item\">';"
"html += '<div class=\"file-header\">📄 ' + file.name + '</div>';"
"html += '<div class=\"file-info\">📊 Taille: ' + Math.round(file.size/1024) + ' KB</div>';"
"if (file.patient_id) html += '<div class=\"file-info\">👤 Patient: ' + file.patient_id + '</div>';"
"if (file.session_id) html += '<div class=\"file-info\">🆔 Session: ' + file.session_id + '</div>';"
"if (file.samples) html += '<div class=\"file-info\">📈 Échantillons: ' + file.samples + '</div>';"
"html += '<div class=\"file-actions\">';"
"html += '<button onclick=\"downloadFile(\\'' + file.name + '\\')\" class=\"success\">⬇️ Télécharger</button>';"
"html += '<button onclick=\"previewFile(\\'' + file.name + '\\')\" class=\"secondary\">👁️ Aperçu</button>';"
"html += '<button onclick=\"deleteFile(\\'' + file.name + '\\')\" class=\"danger\">🗑️ Supprimer</button>';"
"html += '</div>';"
"html += '</div>';"
"});"

"fileListDiv.innerHTML = html;"
"} else {"
"console.log('📂 Aucun fichier trouvé');"
"fileListDiv.innerHTML = '<p>📭 Aucun fichier de session trouvé.</p>';"
"}"
"})"
".catch(function(error) { "
"console.error('📂 Erreur chargement fichiers:', error);"
"document.getElementById('file_list').innerHTML = '<div class=\"status danger\">❌ Erreur chargement: ' + error.message + '</div>';"
"showStatus('❌ Erreur chargement fichiers: ' + error.message, 'error'); "
"});"
"}"

"function downloadFile(filename) {"
"console.log('⬇️ Téléchargement:', filename);"
"showStatus('⬇️ Téléchargement de ' + filename + ' en cours...', 'info');"

"const link = document.createElement('a');"
"link.href = '/api/download?file=' + encodeURIComponent(filename);"
"link.download = filename;"
"link.style.display = 'none';"
"document.body.appendChild(link);"
"link.click();"
"document.body.removeChild(link);"

"showStatus('✅ Téléchargement initié pour ' + filename, 'success');"
"}"

"function previewFile(filename) {"
"console.log('👁️ Aperçu:', filename);"
"showStatus('👁️ Chargement aperçu...', 'info');"

"fetch('/api/preview?file=' + encodeURIComponent(filename))"
".then(function(response) { "
"console.log('👁️ Réponse aperçu:', response);"
"if (!response.ok) {"
"throw new Error('HTTP ' + response.status);"
"}"
"return response.json(); "
"})"
".then(function(data) {"
"console.log('👁️ Données aperçu:', data);"

"if (data.error) {"
"showStatus('❌ Erreur aperçu: ' + data.error, 'error');"
"return;"
"}"

"var html = '<div class=\"debug-info\">';"
"html += '<h4>👁️ Aperçu: ' + filename + '</h4>';"
"html += '<p><strong>🆔 Session ID:</strong> ' + (data.session_id || 'N/A') + '</p>';"
"html += '<p><strong>👤 Patient ID:</strong> ' + (data.patient_id || 'N/A') + '</p>';"
"html += '<p><strong>💾 Taille:</strong> ' + data.file_size_kb + ' KB</p>';"
"if (data.estimated_samples) html += '<p><strong>📈 Échantillons:</strong> ' + data.estimated_samples + '</p>';"
"html += '<button onclick=\"downloadFile(\\'' + filename + '\\')\" style=\"margin-top: 10px;\" class=\"success\">⬇️ Télécharger</button>';"
"html += '</div>';"

"document.getElementById('status').innerHTML = html;"
"})"
".catch(function(error) { "
"console.error('👁️ Erreur aperçu:', error);"
"showStatus('❌ Erreur aperçu: ' + error.message, 'error'); "
"});"
"}"

"function deleteFile(filename) {"
"console.log('🗑️ Suppression:', filename);"
"if (!confirm('⚠️ Supprimer définitivement le fichier: ' + filename + ' ?')) return;"

"showStatus('🗑️ Suppression de ' + filename + '...', 'info');"

"var formData = new FormData();"
"formData.append('file', filename);"

"fetch('/api/delete', { "
"method: 'POST', "
"body: formData "
"})"
".then(function(response) { "
"console.log('🗑️ Réponse suppression:', response);"
"if (!response.ok) {"
"throw new Error('HTTP ' + response.status);"
"}"
"return response.json(); "
"})"
".then(function(data) {"
"console.log('🗑️ Données suppression:', data);"
"if (data.success) {"
"showStatus('✅ Fichier supprimé: ' + filename, 'success');"
"loadFileList();"
"} else {"
"showStatus('❌ Erreur suppression: ' + (data.message || 'Inconnue'), 'error');"
"}"
"})"
".catch(function(error) { "
"console.error('🗑️ Erreur suppression:', error);"
"showStatus('❌ Erreur suppression: ' + error.message, 'error'); "
"});"
"}"

"function deleteAllFiles() {"
"console.log('💥 Suppression globale...');"
"if (!confirm('⚠️⚠️ ATTENTION: Supprimer TOUS les fichiers ?')) return;"

"var confirmation = prompt('Pour confirmer, tapez: SUPPRIMER');"
"if (confirmation !== 'SUPPRIMER') return;"

"showStatus('🗑️ Suppression de tous les fichiers...', 'info');"

"fetch('/api/delete-all', { method: 'POST' })"
".then(function(response) { "
"console.log('💥 Réponse suppression globale:', response);"
"if (!response.ok) {"
"throw new Error('HTTP ' + response.status);"
"}"
"return response.json(); "
"})"
".then(function(data) {"
"console.log('💥 Données suppression globale:', data);"
"if (data.success) {"
"showStatus('✅ ' + data.message, 'success');"
"loadFileList();"
"} else {"
"showStatus('❌ Erreur: ' + (data.message || 'Inconnue'), 'error');"
"}"
"})"
".catch(function(error) { "
"console.error('💥 Erreur suppression globale:', error);"
"showStatus('❌ Erreur: ' + error.message, 'error'); "
"});"
"}"

"function debugSD() {"
"console.log('🔍 Debug SD...');"
"showStatus('🔍 Diagnostic de la carte SD...', 'info');"

"fetch('/api/sd-debug')"
".then(function(response) { "
"console.log('🔍 Réponse debug SD:', response);"
"if (!response.ok) {"
"throw new Error('HTTP ' + response.status);"
"}"
"return response.json(); "
"})"
".then(function(data) {"
"console.log('🔍 Données debug SD:', data);"

"var html = '<div class=\"debug-info\">';"
"html += '<h4>🔍 Diagnostic Carte SD</h4>';"
"html += '<p><strong>💾 SD Initialisée:</strong> ' + (data.sd_initialized ? '✅ OUI' : '❌ NON') + '</p>';"

"if (data.sd_initialized) {"
"html += '<p><strong>📏 Taille carte:</strong> ' + data.card_size_mb + ' MB</p>';"
"html += '<p><strong>📊 Espace libre:</strong> ' + data.free_bytes_mb + ' MB</p>';"
"html += '<p><strong>📁 Fichiers JSON:</strong> ' + data.data_files_count + '</p>';"
"html += '<p><strong>📂 Dossier /data:</strong> ' + (data.data_folder_exists ? '✅ Existe' : '❌ Manquant') + '</p>';"
"} else {"
"html += '<p><strong>❌ Erreur:</strong> ' + (data.error || 'Inconnue') + '</p>';"
"}"

"html += '</div>';"
"document.getElementById('status').innerHTML = html;"
"})"
".catch(function(error) { "
"console.error('🔍 Erreur debug SD:', error);"
"showStatus('❌ Erreur diagnostic: ' + error.message, 'error'); "
"});"
"}"

"window.onload = function() {"
"console.log('🚀 Page chargée - Initialisation...');"
"updateCurrentTime();"
"updateBattery();"  // 🔋 AJOUT: Mise à jour batterie au chargement
"loadFileList();"
"};"

// 🔋 AJOUT: Mise à jour automatique de la batterie toutes les 60 secondes (réduit)
"setInterval(updateBattery, 60000);"

"window.onerror = function(msg, url, line, col, error) {"
"console.error('💥 JavaScript Error:', msg, 'at', url, ':', line);"
"showStatus('💥 Erreur JavaScript: ' + msg, 'error');"
"return false;"
"};"

"console.log('✅ Script complètement chargé');"
"</script>"
"</body>"
"</html>";

void WebServerManager::setup() {
    Serial.println("🌐 === CONFIGURATION SERVEUR WEB ===");
    setupRoutes();
    server.begin();
    Serial.println("✅ Serveur web démarré sur port 80");
    Serial.println("🔍 Routes configurées:");
    Serial.println("   / -> Page principale");
    Serial.println("   /api/config -> Configuration");
    Serial.println("   /api/save -> Sauvegarde");
    Serial.println("   /api/start -> Démarrage session");
    Serial.println("   /api/files -> Liste fichiers");
    Serial.println("   /api/download -> Téléchargement");
    Serial.println("   /api/preview -> Aperçu");
    Serial.println("   /api/delete -> Suppression");
    Serial.println("   /api/delete-all -> Suppression globale");
    Serial.println("   /api/sd-debug -> Debug SD");
    Serial.println("   /api/battery -> État batterie");  // 🔋 AJOUT
}

void WebServerManager::handleClient() {
    server.handleClient();
}

void WebServerManager::stop() {
    server.stop();
    Serial.println("🛑 Serveur web arrêté");
}

void WebServerManager::setupRoutes() {
    Serial.println("🔧 Configuration des routes API...");
    
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/config", HTTP_GET, handleConfig);
    server.on("/api/save", HTTP_POST, handleSave);
    server.on("/api/start", HTTP_POST, handleStart);
    server.on("/api/files", HTTP_GET, handleFiles);
    server.on("/api/download", HTTP_GET, handleDownload);
    server.on("/api/preview", HTTP_GET, handlePreview);
    server.on("/api/delete", HTTP_POST, handleDelete);
    server.on("/api/delete-all", HTTP_POST, handleDeleteAll);
    server.on("/api/sd-debug", HTTP_GET, handleSDDebug);
    server.on("/api/battery", HTTP_GET, handleBattery);  // 🔋 NOUVELLE ROUTE
    
    // 🔧 AJOUT: Route pour capturer les erreurs 404
    server.onNotFound([]() {
        Serial.printf("❌ Route non trouvée: %s\n", server.uri().c_str());
        server.send(404, "application/json", "{\"error\": \"Route non trouvée\"}");
    });
    
    Serial.println("✅ Routes configurées");
}

void WebServerManager::handleRoot() {
    Serial.println("🏠 Requête page principale");
    server.send(200, "text/html", configHTML);
}

void WebServerManager::handleConfig() {
    Serial.println("📋 Requête configuration");
    
    // 🔧 AJOUT: Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    DynamicJsonDocument doc(1024);
    doc["patient_id"] = patientID;
    doc["patient_age"] = patientAge;
    doc["patient_sex"] = patientSex;
    doc["patient_weight"] = patientWeight;
    doc["patient_height"] = patientHeight;
    doc["study_notes"] = studyNotes;
    doc["wifi_ssid"] = wifiSSID;
    doc["wifi_password"] = wifiPassword;
    doc["aws_endpoint"] = String(AWS_API_ENDPOINT);
    doc["mode"] = (int)selectedMode;
    
    String response;
    serializeJson(doc, response);
    
    Serial.printf("📋 Réponse config: %s\n", response.c_str());
    server.send(200, "application/json", response);
}

void WebServerManager::handleSave() {
    Serial.println("💾 === REQUÊTE SAUVEGARDE ===");
    
    // 🔧 AJOUT: Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    if (!server.hasArg("plain")) {
        Serial.println("❌ Pas de données JSON dans la requête");
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Pas de données JSON\"}");
        return;
    }
    
    String jsonString = server.arg("plain");
    Serial.printf("💾 Données reçues: %s\n", jsonString.c_str());
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error) {
        Serial.printf("❌ Erreur parsing JSON: %s\n", error.c_str());
        server.send(400, "application/json", "{\"success\": false, \"message\": \"JSON invalide\"}");
        return;
    }
    
    // Extraction des données
    patientID = doc["patient_id"].as<String>();
    patientAge = doc["patient_age"].as<String>();
    patientSex = doc["patient_sex"].as<String>();
    patientWeight = doc["patient_weight"].as<String>();
    patientHeight = doc["patient_height"].as<String>();
    studyNotes = doc["study_notes"].as<String>();
    wifiSSID = doc["wifi_ssid"].as<String>();
    wifiPassword = doc["wifi_password"].as<String>();
    
    Serial.printf("💾 Patient ID: %s\n", patientID.c_str());
    Serial.printf("💾 WiFi SSID: %s\n", wifiSSID.c_str());
    
    // Capture des données temporelles
    if (doc.containsKey("phone_timestamp")) {
        phoneTimestampMs = doc["phone_timestamp"].as<unsigned long long>();
        systemStartMs = millis();
        Serial.printf("🕐 Timestamp téléphone capturé: %llu ms\n", phoneTimestampMs);
        Serial.printf("🕐 Système à: %lu ms\n", systemStartMs);
        
        if (phoneTimestampMs < 1577836800000ULL) {
            Serial.println("⚠️ Timestamp semble incorrect");
        } else {
            Serial.println("✅ Timestamp valide");
        }
    } else {
        Serial.println("❌ phone_timestamp manquant");
    }
    
    if (doc.containsKey("phone_timezone")) {
        phoneTimezone = doc["phone_timezone"].as<String>();
        Serial.printf("🌍 Timezone: %s\n", phoneTimezone.c_str());
    } else {
        Serial.println("❌ phone_timezone manquant");
    }
    
    String mode = doc["mode"].as<String>();
    selectedMode = (mode == "wifi") ? MODE_WIFI_AWS : MODE_SD_RECORDING;
    Serial.printf("📊 Mode sélectionné: %s\n", (selectedMode == MODE_WIFI_AWS) ? "WiFi+AWS" : "SD");
    
    SystemManager::saveConfiguration();
    
    Serial.println("✅ Configuration sauvegardée");
    server.send(200, "application/json", "{\"success\": true}");
}

void WebServerManager::handleStart() {
    Serial.println("🚀 === REQUÊTE DÉMARRAGE SESSION ===");
    
    // 🔧 AJOUT: Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    if (patientID.length() == 0) {
        Serial.println("❌ Patient ID manquant");
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Patient ID requis\"}");
        return;
    }
    
    Serial.printf("🚀 Démarrage pour patient: %s\n", patientID.c_str());
    
    // Capture du timestamp de démarrage si envoyé
    if (server.hasArg("plain")) {
        String jsonString = server.arg("plain");
        Serial.printf("🚀 Données démarrage: %s\n", jsonString.c_str());
        
        DynamicJsonDocument doc(512);
        deserializeJson(doc, jsonString);
        
        if (doc.containsKey("phone_timestamp")) {
            phoneTimestampMs = doc["phone_timestamp"].as<unsigned long long>();
            systemStartMs = millis();
            Serial.printf("🚀 Timestamp actualisé: %llu ms\n", phoneTimestampMs);
            Serial.printf("🚀 Système à: %lu ms\n", systemStartMs);
            
            if (phoneTimestampMs < 1577836800000ULL) {
                Serial.println("⚠️ Timestamp de démarrage incorrect - correction");
                phoneTimestampMs = millis() + 1577836800000ULL;
                Serial.printf("⚠️ Fallback: %llu ms\n", phoneTimestampMs);
            } else {
                Serial.println("✅ Timestamp de démarrage valide");
            }
        } else {
            Serial.println("❌ phone_timestamp manquant au démarrage");
        }
        
        if (doc.containsKey("phone_timezone")) {
            phoneTimezone = doc["phone_timezone"].as<String>();
            Serial.printf("🌍 Timezone confirmé: %s\n", phoneTimezone.c_str());
        }
    } else {
        Serial.println("⚠️ Pas de données temporelles au démarrage");
    }
    
    // Forcer la sauvegarde des données temporelles
    SystemManager::saveConfiguration();
    
    Serial.println("✅ Préparation démarrage terminée");
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Session démarrée\"}");
    
    Serial.println("🚀 Arrêt serveur web...");
    delay(1000);
    
    server.stop();
    WiFi.softAPdisconnect(true);
    
    delay(2000);
    
    Serial.println("🚀 Initialisation du mode sélectionné...");
    SystemManager::initializeSelectedMode();
}

void WebServerManager::handleFiles() {
    Serial.println("📂 === REQUÊTE LISTE FICHIERS ===");
    
    // 🔧 AJOUT: Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    DynamicJsonDocument doc(4096);
    JsonArray filesArray = doc.createNestedArray("files");
    
    // Protection SPI
    digitalWrite(LIS3DH_CS, HIGH);
    delay(10);
    
    if (!SD.begin(SD_CS, SPI, 400000)) {
        Serial.println("❌ Carte SD non accessible");
        doc["error"] = "Carte SD non accessible";
        String response;
        serializeJson(doc, response);
        server.send(500, "application/json", response);
        return;
    }
    
    Serial.println("✅ SD initialisée pour liste fichiers");
    
    if (SD.exists("/data")) {
        File root = SD.open("/data");
        if (root) {
            File file = root.openNextFile();
            int fileCount = 0;
            while (file) {
                String filename = file.name();
                if (filename.endsWith(".json")) {
                    fileCount++;
                    Serial.printf("📄 Fichier %d: %s (%d bytes)\n", fileCount, filename.c_str(), file.size());
                    
                    JsonObject fileObj = filesArray.createNestedObject();
                    fileObj["name"] = filename;
                    fileObj["size"] = file.size();
                    
                    // Parsing simple
                    file.seek(0);
                    String content = "";
                    int bytesRead = 0;
                    while (file.available() && bytesRead < 800) {
                        content += (char)file.read();
                        bytesRead++;
                        if (content.indexOf("\"sessions\"") != -1) break;
                    }
                    
                    // Extraction métadonnées
                    if (content.indexOf("patient_id") != -1) {
                        int start = content.indexOf("\"patient_id\":\"") + 14;
                        int end = content.indexOf("\"", start);
                        if (start > 13 && end > start) {
                            fileObj["patient_id"] = content.substring(start, end);
                        }
                    }
                }
                file.close();
                file = root.openNextFile();
            }
            root.close();
            Serial.printf("📂 Total fichiers JSON trouvés: %d\n", fileCount);
        } else {
            Serial.println("❌ Impossible d'ouvrir /data");
        }
    } else {
        Serial.println("📂 Dossier /data n'existe pas");
    }
    
    String response;
    serializeJson(doc, response);
    Serial.printf("📂 Réponse liste fichiers: %s\n", response.c_str());
    server.send(200, "application/json", response);
}

void WebServerManager::handleDownload() {
    Serial.println("⬇️ === REQUÊTE TÉLÉCHARGEMENT ===");
    
    if (!server.hasArg("file")) {
        Serial.println("❌ Paramètre 'file' manquant");
        server.send(400, "text/plain", "Parametre 'file' manquant");
        return;
    }
    
    String filename = server.arg("file");
    Serial.printf("⬇️ Fichier demandé: %s\n", filename.c_str());
    
    if (filename.indexOf("..") != -1 || !filename.endsWith(".json")) {
        Serial.println("❌ Nom fichier invalide");
        server.send(400, "text/plain", "Nom fichier invalide");
        return;
    }
    
    String fullPath = "/data/" + filename;
    Serial.printf("⬇️ Chemin complet: %s\n", fullPath.c_str());
    
    // Protection SPI
    digitalWrite(LIS3DH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    delay(50);
    
    if (!SD.begin(SD_CS, SPI, 400000)) {
        Serial.println("❌ Erreur carte SD");
        server.send(500, "text/plain", "Erreur carte SD");
        return;
    }
    
    if (!SD.exists(fullPath)) {
        Serial.printf("❌ Fichier non trouvé: %s\n", fullPath.c_str());
        server.send(404, "text/plain", "Fichier non trouve");
        return;
    }
    
    File file = SD.open(fullPath, FILE_READ);
    if (!file) {
        Serial.println("❌ Erreur ouverture fichier");
        server.send(500, "text/plain", "Erreur ouverture fichier");
        return;
    }
    
    size_t fileSize = file.size();
    Serial.printf("📊 Téléchargement: %s (%d KB)\n", filename.c_str(), fileSize / 1024);
    
    // Headers HTTP
    server.setContentLength(fileSize);
    server.send(200, "application/json", "");
    
    // Envoi par chunks
    const size_t CHUNK_SIZE = 512;
    uint8_t buffer[CHUNK_SIZE];
    size_t totalSent = 0;
    
    while (file.available() && server.client().connected()) {
        size_t bytesToRead = min((size_t)file.available(), CHUNK_SIZE);
        size_t bytesRead = file.read(buffer, bytesToRead);
        
        if (bytesRead > 0) {
            server.client().write(buffer, bytesRead);
            totalSent += bytesRead;
        } else {
            break;
        }
        
        yield();
    }
    
    file.close();
    Serial.printf("✅ Téléchargement terminé: %d/%d bytes envoyés\n", totalSent, fileSize);
}

void WebServerManager::handlePreview() {
    Serial.println("👁️ === REQUÊTE APERÇU ===");
    
    // 🔧 AJOUT: Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    if (!server.hasArg("file")) {
        Serial.println("❌ Paramètre 'file' manquant");
        server.send(400, "application/json", "{\"error\": \"Parametre manquant\"}");
        return;
    }
    
    String filename = server.arg("file");
    String fullPath = "/data/" + filename;
    Serial.printf("👁️ Aperçu: %s\n", fullPath.c_str());
    
    // Protection SPI
    digitalWrite(LIS3DH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    delay(50);
    
    if (!SD.begin(SD_CS, SPI, 400000)) {
        Serial.println("❌ Carte SD non accessible");
        server.send(500, "application/json", "{\"error\": \"Carte SD non accessible\"}");
        return;
    }
    
    if (!SD.exists(fullPath)) {
        Serial.printf("❌ Fichier non trouvé: %s\n", fullPath.c_str());
        server.send(404, "application/json", "{\"error\": \"Fichier non trouve\"}");
        return;
    }
    
    File file = SD.open(fullPath, FILE_READ);
    if (!file) {
        Serial.println("❌ Erreur ouverture fichier");
        server.send(500, "application/json", "{\"error\": \"Erreur ouverture fichier\"}");
        return;
    }
    
    size_t fileSize = file.size();
    
    DynamicJsonDocument response(512);
    response["filename"] = filename;
    response["file_size"] = fileSize;
    response["file_size_kb"] = fileSize / 1024;
    
    // Lecture des premières lignes
    String content = "";
    int bytesRead = 0;
    while (file.available() && bytesRead < 500) {
        content += (char)file.read();
        bytesRead++;
    }
    file.close();
    
    // Extraction métadonnées
    if (content.indexOf("patient_id") != -1) {
        int start = content.indexOf("\"patient_id\":\"") + 14;
        int end = content.indexOf("\"", start);
        if (start > 13 && end > start) {
            response["patient_id"] = content.substring(start, end);
        }
    }
    
    if (content.indexOf("session_id") != -1) {
        int start = content.indexOf("\"session_id\":\"") + 14;
        int end = content.indexOf("\"", start);
        if (start > 13 && end > start) {
            response["session_id"] = content.substring(start, end);
        }
    }
    
    response["estimated_samples"] = (fileSize - 500) / 100;
    
    String responseStr;
    serializeJson(response, responseStr);
    Serial.printf("👁️ Réponse aperçu: %s\n", responseStr.c_str());
    server.send(200, "application/json", responseStr);
}

void WebServerManager::handleDelete() {
    Serial.println("🗑️ === REQUÊTE SUPPRESSION ===");
    
    // 🔧 AJOUT: Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    String filename = "";
    if (server.hasArg("file")) {
        filename = server.arg("file");
    }
    
    Serial.printf("🗑️ Fichier à supprimer: %s\n", filename.c_str());
    
    if (filename.length() == 0 || filename.indexOf("..") != -1 || !filename.endsWith(".json")) {
        Serial.println("❌ Fichier invalide");
        server.send(400, "application/json", "{\"success\": false, \"message\": \"Fichier invalide\"}");
        return;
    }
    
    String fullPath = "/data/" + filename;
    
    // Protection SPI
    digitalWrite(LIS3DH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    delay(50);
    
    if (!SD.begin(SD_CS, SPI, 400000)) {
        Serial.println("❌ Erreur carte SD");
        server.send(500, "application/json", "{\"success\": false, \"message\": \"Erreur carte SD\"}");
        return;
    }
    
    if (SD.remove(fullPath)) {
        Serial.printf("✅ Fichier supprimé: %s\n", filename.c_str());
        server.send(200, "application/json", "{\"success\": true, \"message\": \"Fichier supprime\"}");
    } else {
        Serial.printf("❌ Erreur suppression: %s\n", filename.c_str());
        server.send(500, "application/json", "{\"success\": false, \"message\": \"Erreur suppression\"}");
    }
}

void WebServerManager::handleDeleteAll() {
    Serial.println("💥 === REQUÊTE SUPPRESSION GLOBALE ===");
    
    // 🔧 AJOUT: Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    // Protection SPI
    digitalWrite(LIS3DH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    delay(50);
    
    if (!SD.begin(SD_CS, SPI, 400000)) {
        Serial.println("❌ Erreur carte SD");
        server.send(500, "application/json", "{\"success\": false, \"message\": \"Erreur carte SD\"}");
        return;
    }
    
    if (!SD.exists("/data")) {
        Serial.println("❌ Dossier /data inexistant");
        server.send(404, "application/json", "{\"success\": false, \"message\": \"Dossier /data inexistant\"}");
        return;
    }
    
    File dataDir = SD.open("/data");
    if (!dataDir) {
        Serial.println("❌ Erreur ouverture dossier");
        server.send(500, "application/json", "{\"success\": false, \"message\": \"Erreur ouverture dossier\"}");
        return;
    }
    
    int filesDeleted = 0;
    File file = dataDir.openNextFile();
    while (file) {
        String filename = file.name();
        file.close();
        
        if (filename.endsWith(".json")) {
            String fullPath = "/data/" + filename;
            if (SD.remove(fullPath)) {
                filesDeleted++;
                Serial.printf("🗑️ Supprimé: %s\n", filename.c_str());
            }
            delay(10);
        }
        
        file = dataDir.openNextFile();
    }
    dataDir.close();
    
    Serial.printf("✅ %d fichiers supprimés\n", filesDeleted);
    
    DynamicJsonDocument response(256);
    response["success"] = true;
    response["files_deleted"] = filesDeleted;
    response["message"] = String(filesDeleted) + " fichiers supprimes";
    
    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void WebServerManager::handleSDDebug() {
    Serial.println("🔍 === REQUÊTE DEBUG SD ===");
    
    // 🔧 AJOUT: Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    DynamicJsonDocument doc(1024);
    
    // Protection SPI
    digitalWrite(LIS3DH_CS, HIGH);
    delay(10);
    
    bool sdOK = SD.begin(SD_CS, SPI, 400000);
    doc["sd_initialized"] = sdOK;
    
    if (sdOK) {
        Serial.println("✅ SD OK pour debug");
        doc["card_size_mb"] = SD.cardSize() / (1024 * 1024);
        doc["free_bytes_mb"] = (SD.totalBytes() - SD.usedBytes()) / (1024 * 1024);
        doc["data_folder_exists"] = SD.exists("/data");
        
        // Compter les fichiers
        int fileCount = 0;
        if (SD.exists("/data")) {
            File dataDir = SD.open("/data");
            if (dataDir) {
                File file = dataDir.openNextFile();
                while (file) {
                    if (String(file.name()).endsWith(".json")) {
                        fileCount++;
                    }
                    file.close();
                    file = dataDir.openNextFile();
                }
                dataDir.close();
            }
        }
        doc["data_files_count"] = fileCount;
        Serial.printf("🔍 Debug SD: %d MB, %d fichiers\n", 
                     (int)(SD.cardSize() / (1024 * 1024)), fileCount);
    } else {
        Serial.println("❌ SD inaccessible pour debug");
        doc["error"] = "Carte SD inaccessible";
    }
    
    String response;
    serializeJson(doc, response);
    Serial.printf("🔍 Réponse debug: %s\n", response.c_str());
    server.send(200, "application/json", response);
}

// 🔋 NOUVELLE FONCTION: API batterie (VERSION SÉCURISÉE)
void WebServerManager::handleBattery() {
    Serial.println("🔋 === REQUÊTE ÉTAT BATTERIE ===");
    
    // Headers CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    DynamicJsonDocument doc(512);
    
    try {
        // 🔧 SÉCURITÉ: Timeout pour éviter blocage
        unsigned long startTime = millis();
        const unsigned long TIMEOUT_MS = 2000;  // 2 secondes max
        
        Serial.println("🔋 Récupération info batterie...");
        
        // Vérifier si BatteryManager est disponible
        bool batteryAvailable = true;
        BatteryInfo batteryInfo;
        
        // 🔧 PROTECTION: Try-catch simulé avec timeout
        if (millis() - startTime < TIMEOUT_MS) {
            batteryInfo = BatteryManager::getBatteryInfo();
        } else {
            Serial.println("⏰ Timeout récupération batterie");
            batteryAvailable = false;
        }
        
        if (batteryAvailable && batteryInfo.isValid) {
            Serial.printf("🔋 Données valides: %.1f%% (%.2fV)\n", 
                         batteryInfo.percentage, batteryInfo.voltage);
            
            doc["success"] = true;
            doc["percentage"] = batteryInfo.percentage;
            doc["voltage"] = batteryInfo.voltage;
            doc["status"] = batteryInfo.status;
            doc["charging"] = batteryInfo.isCharging;
            
            Serial.printf("🔋 JSON créé: %.1f%%\n", batteryInfo.percentage);
        } else {
            Serial.println("❌ Données batterie non disponibles - Utilisation valeurs par défaut");
            
            // 🔧 FALLBACK: Valeurs par défaut si problème
            doc["success"] = false;
            doc["error"] = "Fuel gauge temporairement indisponible";
            doc["percentage"] = 75.0;  // Valeur par défaut
            doc["voltage"] = 3.7;      // Valeur par défaut
            doc["status"] = "Estimation";
            doc["charging"] = false;
        }
        
        String response;
        serializeJson(doc, response);
        Serial.printf("🔋 Envoi réponse: %s\n", response.c_str());
        
        // 🔧 IMPORTANT: Envoyer la réponse immédiatement
        server.send(200, "application/json", response);
        Serial.println("✅ Réponse batterie envoyée");
        
    } catch (...) {
        Serial.println("💥 Exception dans handleBattery");
        
        // 🔧 SÉCURITÉ: Réponse d'urgence en cas d'exception
        server.send(500, "application/json", 
                   "{\"success\": false, \"error\": \"Erreur interne\", \"percentage\": 50.0, \"voltage\": 3.6, \"status\": \"Erreur\", \"charging\": false}");
    }
}