let r = typeof 1 + typeof 'a' + typeof {}; let n = 0; while (n < 3) { n++; if (n === 2) { continue; } } r + n;
