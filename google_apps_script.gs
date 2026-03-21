/**
 * Google Apps Script — Attendance Logger
 *
 * HOW TO SET UP:
 * 1. Go to https://sheets.google.com and create a new spreadsheet.
 * 2. Name it "Attendance" (or anything you like).
 * 3. In the spreadsheet, go to Extensions > Apps Script.
 * 4. Delete the default code and paste this entire file.
 * 5. Click Save, then click Deploy > New Deployment.
 * 6. Type: Web App
 *    Execute as: Me
 *    Who has access: Anyone
 * 7. Click Deploy, authorize when prompted.
 * 8. Copy the Web App URL and paste it into SHEETS_URL in the .ino file.
 */

// The name of the sheet tab where attendance will be logged
const SHEET_NAME = "Attendance";

// ── Called by ESP32 via HTTP GET ───────────────────────────────────────────
function doGet(e) {
  try {
    var name  = e.parameter.name  || "Unknown";
    var roll  = e.parameter.roll  || "N/A";
    var fpid  = e.parameter.fpid  || "0";

    logAttendance(name, roll, fpid);

    return ContentService
      .createTextOutput("OK")
      .setMimeType(ContentService.MimeType.TEXT);

  } catch (err) {
    return ContentService
      .createTextOutput("ERROR: " + err.message)
      .setMimeType(ContentService.MimeType.TEXT);
  }
}

// ── Append a row to the sheet ──────────────────────────────────────────────
function logAttendance(name, roll, fpid) {
  var ss    = SpreadsheetApp.getActiveSpreadsheet();
  var sheet = ss.getSheetByName(SHEET_NAME);

  // Create the sheet if it doesn't exist yet
  if (!sheet) {
    sheet = ss.insertSheet(SHEET_NAME);
    // Add header row
    sheet.appendRow(["Timestamp", "Name", "Roll No", "Fingerprint ID", "Status"]);
    sheet.getRange(1, 1, 1, 5).setFontWeight("bold");
  }

  var timestamp = Utilities.formatDate(
    new Date(),
    Session.getScriptTimeZone(),
    "yyyy-MM-dd HH:mm:ss"
  );

  // Check if already marked present today (prevent duplicate entries)
  var today     = Utilities.formatDate(new Date(), Session.getScriptTimeZone(), "yyyy-MM-dd");
  var data      = sheet.getDataRange().getValues();
  var duplicate = false;

  for (var i = 1; i < data.length; i++) {
    var rowDate = Utilities.formatDate(new Date(data[i][0]), Session.getScriptTimeZone(), "yyyy-MM-dd");
    if (data[i][2] == roll && rowDate == today) {
      duplicate = true;
      break;
    }
  }

  if (duplicate) {
    // Update last column to note duplicate attempt
    sheet.appendRow([timestamp, name, roll, fpid, "Already Marked"]);
  } else {
    sheet.appendRow([timestamp, name, roll, fpid, "Present"]);
  }
}
