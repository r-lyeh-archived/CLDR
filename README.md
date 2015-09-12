# CLDR
Compact data from the Unicode Common Locale Data Repository

For anyone interested, I just dumped most of the CLDR data in a compact way (see [provided CLDR.INI file](cldr.ini)).

The final data for all languages is 13,727,497 bytes, but still highly compressible, as seen below.

```lisp
BSC:    1,023,137 bytes, ratio=92.5468% enctime=1211822us dectime=757387us 
BROTLI: 1,287,148 bytes, ratio=90.6236% enctime=67236011us dectime=50243us 
LZMA25: 1,369,212 bytes, ratio=90.0258% enctime=3961970us dectime=98609us 
LZIP:   1,369,811 bytes, ratio=90.0214% enctime=3895218us dectime=131528us 
LZMA20: 1,423,667 bytes, ratio=89.6291% enctime=3334007us dectime=103905us 
MINIZ:  1,892,977 bytes, ratio=86.2103% enctime=697077us dectime=31830us 
ZSTD:   2,108,053 bytes, ratio=84.6436% enctime=65694us dectime=34525us 
LZ4HC:  2,151,652 bytes, ratio=84.326% enctime=491871us dectime=13641us 
LZ4:    2,918,991 bytes, ratio=78.7362% enctime=37851us dectime=13775us 
RAW:   13,727,497 bytes, ratio=0% enctime=16242us dectime=7658us 
```

This is what is currently processed from the CLDR repos:
- [ ] skipped
- [x] extracted

cldr-core/supplemental/
- [ ] aliases.json
- [ ] calendarData.json
- [ ] calendarPreferenceData.json
- [x] characterFallbacks.json
- [ ] codeMappings.json
- [x] currencyData.json
- [x] gender.json
- [ ] languageData.json
- [ ] languageMatching.json
- [ ] likelySubtags.json
- [x] measurementData.json
- [ ] metaZones.json
- [ ] numberingSystems.json
- [x] ordinals.json
- [x] parentLocales.json
- [x] plurals.json
- [ ] primaryZones.json
- [ ] references.json
- [x] telephoneCodeData.json
- [ ] territoryContainment.json
- [ ] territoryInfo.json (interesting!)
- [x] timeData.json
- [x] weekData.json
- [x] windowsZones.json

cldr-dates-modern\main\xx-XX
- [ ] ca-generic.json
- [x] ca-gregorian.json
- [x] dateFields.json
- [x] timeZoneNames.json

cldr-localenames-modern\main\xx-XX
- [x] languages.json
- [ ] localeDisplayNames.json
- [ ] scripts.json
- [x] territories.json
- [ ] transformNames.json
- [ ] variants.json

cldr-misc-modern\main\xx-XX
- [x] characters.json
- [x] contextTransforms.json
- [x] delimiters.json
- [x] layout.json
- [x] listPatterns.json
- [ ] posix.json

cldr-numbers-modern\main\xx-XX
- [x] currencies.json
- [ ] numbers.json

cldr-segments-modern\segments\xx-XX
- [ ] suppressions.json

## Licenses
- [CLDR](https://github.com/r-lyeh/cldr), public domain.
- http://unicode.org/copyright.html, original license for the data. 
