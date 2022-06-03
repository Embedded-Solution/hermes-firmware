#ifndef CONNECT_HPP
#define CONNECT_HPP
#include <Wake.hpp>
#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <AutoConnect.h>
#include <Storage/SecureDigital.hpp>
#include <Navigation/GNSS.hpp>
#include <Utils.hpp>

#define SUCCESS 0
#define OLD_FIRMWARE_ERROR -1 
#define FIRMWARE_SIZE_ERROR -2
#define GET_FIRMWARE_ERROR -3
#define CONNECTION_ERROR -4
#define HTTP_BEGIN_ERROR -5


void startPortal(SecureDigital sd);

int uploadDives(SecureDigital sd);

int ota();

int post(String records, bool metadata = 0);

const String indexPath = "/index.json";
const String metadataURL = "https://project-hermes.azurewebsites.net/api/Remora";
const String recordURL = "https://project-hermes.azurewebsites.net/api/RemoraRecord";

/*const static char *test_root_ca PROGMEM =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIIujCCBqKgAwIBAgITMwAxS6DhmVCLBf6MWwAAADFLoDANBgkqhkiG9w0BAQwF\n"
    "ADBZMQswCQYDVQQGEwJVUzEeMBwGA1UEChMVTWljcm9zb2Z0IENvcnBvcmF0aW9u\n"
    "MSowKAYDVQQDEyFNaWNyb3NvZnQgQXp1cmUgVExTIElzc3VpbmcgQ0EgMDEwHhcN\n"
    "MjIwMzE0MTgzOTU1WhcNMjMwMzA5MTgzOTU1WjBqMQswCQYDVQQGEwJVUzELMAkG\n"
    "A1UECBMCV0ExEDAOBgNVBAcTB1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBD\n"
    "b3Jwb3JhdGlvbjEcMBoGA1UEAwwTKi5henVyZXdlYnNpdGVzLm5ldDCCASIwDQYJ\n"
    "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAM3heDMqn7v8cmh4A9vECuEfuiUnKBIw\n"
    "7y0Sf499Z7WW92HDkIvV3eJ6jcyq41f2UJcG8ivCu30eMnYyyI+aRHIedkvOBA2i\n"
    "PqG78e99qGTuKCj9lrJGVfeTBJ1VIlPvfuHFv/3JaKIBpRtuqxCdlgsGAJQmvHEn\n"
    "vIHUV2jgj4iWNBDoC83ShtWg6qV2ol7yiaClB20Af5byo36jVdMN6vS+/othn3jG\n"
    "pn+NP00DWYbP5y4qhs5XLH9wQZaTUPKIaUxmHewErcM0rMAaWl8wMqQTeNYf3l5D\n"
    "ax50yuEg9VVjtbDdSmvOkslGpVqsOl1NrmyN7gCvcvcRUQcxIiXJQc0CAwEAAaOC\n"
    "BGgwggRkMIIBfwYKKwYBBAHWeQIEAgSCAW8EggFrAWkAdgCt9776fP8QyIudPZwe\n"
    "PhhqtGcpXc+xDCTKhYY069yCigAAAX+Jw/reAAAEAwBHMEUCIE8AAjvwO4AffPn7\n"
    "un67WykJ2hGB4n8qJE7pk4QYjWW+AiEA/pio1E9ALt30Kh/Ga4gRefH1ILbQ8n4h\n"
    "bHFatezIcvYAdwB6MoxU2LcttiDqOOBSHumEFnAyE4VNO9IrwTpXo1LrUgAAAX+J\n"
    "w/qlAAAEAwBIMEYCIQCdbj6FOX6wK+dLoqjWKuCgkKSsZsJKpVik6HjlRgomzQIh\n"
    "AM7mYp5dBFmNLas3fFcP0rMMK+17n8u0GhFH2KpkPr1SAHYA6D7Q2j71BjUy51co\n"
    "vIlryQPTy9ERa+zraeF3fW0GvW4AAAF/icP6jgAABAMARzBFAiAhjTz3PBjqRrpY\n"
    "eH7us44lESC7c0dzdTcehTeAwmEyrgIhAOCaqmqA+ercv+39jzFWkctG36bazRFX\n"
    "4gGNiKU0bctcMCcGCSsGAQQBgjcVCgQaMBgwCgYIKwYBBQUHAwIwCgYIKwYBBQUH\n"
    "AwEwPAYJKwYBBAGCNxUHBC8wLQYlKwYBBAGCNxUIh73XG4Hn60aCgZ0ujtAMh/Da\n"
    "HV2ChOVpgvOnPgIBZAIBJTCBrgYIKwYBBQUHAQEEgaEwgZ4wbQYIKwYBBQUHMAKG\n"
    "YWh0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2lvcHMvY2VydHMvTWljcm9zb2Z0\n"
    "JTIwQXp1cmUlMjBUTFMlMjBJc3N1aW5nJTIwQ0ElMjAwMSUyMC0lMjB4c2lnbi5j\n"
    "cnQwLQYIKwYBBQUHMAGGIWh0dHA6Ly9vbmVvY3NwLm1pY3Jvc29mdC5jb20vb2Nz\n"
    "cDAdBgNVHQ4EFgQUiiks5RXI6IIQccflfDtgAHndN7owDgYDVR0PAQH/BAQDAgSw\n"
    "MHwGA1UdEQR1MHOCEyouYXp1cmV3ZWJzaXRlcy5uZXSCFyouc2NtLmF6dXJld2Vi\n"
    "c2l0ZXMubmV0ghIqLmF6dXJlLW1vYmlsZS5uZXSCFiouc2NtLmF6dXJlLW1vYmls\n"
    "ZS5uZXSCFyouc3NvLmF6dXJld2Vic2l0ZXMubmV0MAwGA1UdEwEB/wQCMAAwZAYD\n"
    "VR0fBF0wWzBZoFegVYZTaHR0cDovL3d3dy5taWNyb3NvZnQuY29tL3BraW9wcy9j\n"
    "cmwvTWljcm9zb2Z0JTIwQXp1cmUlMjBUTFMlMjBJc3N1aW5nJTIwQ0ElMjAwMS5j\n"
    "cmwwZgYDVR0gBF8wXTBRBgwrBgEEAYI3TIN9AQEwQTA/BggrBgEFBQcCARYzaHR0\n"
    "cDovL3d3dy5taWNyb3NvZnQuY29tL3BraW9wcy9Eb2NzL1JlcG9zaXRvcnkuaHRt\n"
    "MAgGBmeBDAECAjAfBgNVHSMEGDAWgBQPIF3XoVeV25LPK9DHwncEznKAdjAdBgNV\n"
    "HSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwDQYJKoZIhvcNAQEMBQADggIBAKtk\n"
    "4nEDfqxbP80uaoBoPaeeX4G/tBNcfpR2sf6soW8atAqGOohdLPcE0n5/KJn+H4u7\n"
    "CsZdTJyUVxBxAlpqAc9JABl4urWNbhv4pueGBZXOn5K5Lpup/gp1HhCx4XKFno/7\n"
    "T22NVDol4LRLUTeTkrpNyYLU5QYBQpqlFMAcvem/2seiPPYghFtLr5VWVEikUvnf\n"
    "wSlECNk84PT7mOdbrX7T3CbG9WEZVmSYxMCS4pwcW3caXoSzUzZ0H1sJndCJW8La\n"
    "9tekRKkMVkN558S+FFwaY1yARNqCFeK+yiwvkkkojqHbgwFJgCFWYy37kFR9uPiv\n"
    "3sTHvs8IZ5K8TY7rHk3pSMYqoBTODCs7wKGiByWSDMcfAgGBzjt95SKfq0p6sj0C\n"
    "+HWFiyKR+PTi2esFP9Vr9sC9jfRM6zwa7KnONqLefHauJPdNMt5l1FQGWvyco4IN\n"
    "lwK3Z9FfEOFZA4YcjsqnkNacKZqLjgis3FvD8VPXETgRuffVc75lJxH6WmkwqdXj\n"
    "BlU8wOcJyXTmM1ehYpziCpWvGBSEIsFuK6BC/iBnQEuWKdctAdbHIDlLctGgDWjx\n"
    "xYDPZ/TtORGL8YaDnj6QHeOURIAHCtt6NCWKV6OR2HtMx+tCEvfi5ION1dyJ9hAX\n"
    "+4K9FXc71ab7tdV/GLPkWc8Q0x1nk7ogDYcqKbiF\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIIF8zCCBNugAwIBAgIQCq+mxcpjxFFB6jvh98dTFzANBgkqhkiG9w0BAQwFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
    "MjAeFw0yMDA3MjkxMjMwMDBaFw0yNDA2MjcyMzU5NTlaMFkxCzAJBgNVBAYTAlVT\n"
    "MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xKjAoBgNVBAMTIU1pY3Jv\n"
    "c29mdCBBenVyZSBUTFMgSXNzdWluZyBDQSAwMTCCAiIwDQYJKoZIhvcNAQEBBQAD\n"
    "ggIPADCCAgoCggIBAMedcDrkXufP7pxVm1FHLDNA9IjwHaMoaY8arqqZ4Gff4xyr\n"
    "RygnavXL7g12MPAx8Q6Dd9hfBzrfWxkF0Br2wIvlvkzW01naNVSkHp+OS3hL3W6n\n"
    "l/jYvZnVeJXjtsKYcXIf/6WtspcF5awlQ9LZJcjwaH7KoZuK+THpXCMtzD8XNVdm\n"
    "GW/JI0C/7U/E7evXn9XDio8SYkGSM63aLO5BtLCv092+1d4GGBSQYolRq+7Pd1kR\n"
    "EkWBPm0ywZ2Vb8GIS5DLrjelEkBnKCyy3B0yQud9dpVsiUeE7F5sY8Me96WVxQcb\n"
    "OyYdEY/j/9UpDlOG+vA+YgOvBhkKEjiqygVpP8EZoMMijephzg43b5Qi9r5UrvYo\n"
    "o19oR/8pf4HJNDPF0/FJwFVMW8PmCBLGstin3NE1+NeWTkGt0TzpHjgKyfaDP2tO\n"
    "4bCk1G7pP2kDFT7SYfc8xbgCkFQ2UCEXsaH/f5YmpLn4YPiNFCeeIida7xnfTvc4\n"
    "7IxyVccHHq1FzGygOqemrxEETKh8hvDR6eBdrBwmCHVgZrnAqnn93JtGyPLi6+cj\n"
    "WGVGtMZHwzVvX1HvSFG771sskcEjJxiQNQDQRWHEh3NxvNb7kFlAXnVdRkkvhjpR\n"
    "GchFhTAzqmwltdWhWDEyCMKC2x/mSZvZtlZGY+g37Y72qHzidwtyW7rBetZJAgMB\n"
    "AAGjggGtMIIBqTAdBgNVHQ4EFgQUDyBd16FXlduSzyvQx8J3BM5ygHYwHwYDVR0j\n"
    "BBgwFoAUTiJUIBiV5uNu5g/6+rkS7QYXjzkwDgYDVR0PAQH/BAQDAgGGMB0GA1Ud\n"
    "JQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAGAQH/AgEAMHYG\n"
    "CCsGAQUFBwEBBGowaDAkBggrBgEFBQcwAYYYaHR0cDovL29jc3AuZGlnaWNlcnQu\n"
    "Y29tMEAGCCsGAQUFBzAChjRodHRwOi8vY2FjZXJ0cy5kaWdpY2VydC5jb20vRGln\n"
    "aUNlcnRHbG9iYWxSb290RzIuY3J0MHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6Ly9j\n"
    "cmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcmwwN6A1oDOG\n"
    "MWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5j\n"
    "cmwwHQYDVR0gBBYwFDAIBgZngQwBAgEwCAYGZ4EMAQICMBAGCSsGAQQBgjcVAQQD\n"
    "AgEAMA0GCSqGSIb3DQEBDAUAA4IBAQAlFvNh7QgXVLAZSsNR2XRmIn9iS8OHFCBA\n"
    "WxKJoi8YYQafpMTkMqeuzoL3HWb1pYEipsDkhiMnrpfeYZEA7Lz7yqEEtfgHcEBs\n"
    "K9KcStQGGZRfmWU07hPXHnFz+5gTXqzCE2PBMlRgVUYJiA25mJPXfB00gDvGhtYa\n"
    "+mENwM9Bq1B9YYLyLjRtUz8cyGsdyTIG/bBM/Q9jcV8JGqMU/UjAdh1pFyTnnHEl\n"
    "Y59Npi7F87ZqYYJEHJM2LGD+le8VsHjgeWX2CJQko7klXvcizuZvUEDTjHaQcs2J\n"
    "+kPgfyMIOY1DMJ21NxOJ2xPRC/wAh/hzSBRVtoAnyuxtkZ4VjIOh\n"
    "-----END CERTIFICATE-----\n";
*/

#endif