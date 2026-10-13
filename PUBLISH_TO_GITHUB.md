# Publishing cpp-CLP to GitHub

The repo has been **initialized locally only**. No GitHub repository has been created or pushed.

1. On GitHub, create a **private**, **empty** repository named `cpp-CLP` under `Tanhyka`. Do not pre-add README, .gitignore or license.
2. After extracting the source ZIP into a folder, from that folder:

```powershell
git init -b main
git add .
git commit -m "Import Ryuugin CLP C++ rules core (provisional)"
git remote add origin https://github.com/Tanhyka/cpp-CLP.git
git push -u origin main
```

Alternatively, preserve this prepared initial Git commit from `cpp-CLP.bundle`:

```powershell
git clone .\cpp-CLP.bundle cpp-CLP
cd cpp-CLP
git remote add origin https://github.com/Tanhyka/cpp-CLP.git
git push -u origin main
```

**No stable v1.0.0 tag yet**. Keep the repository private until parity tests and license are cleared.
