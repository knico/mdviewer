# Release process

1. Update the version in `package.json` and `adapters/notepad-plus-plus/native/version.rc`.
2. Run the automated checks and the host validation applicable to each architecture.
3. Build the x64 and x86 plugin packages and verify their reported DLL version, architecture, ZIP contents and SHA-256.
4. Publish the source tag and architecture-specific ZIP files in a GitHub Release.
5. Update the Notepad++ Plugin List entry with the matching version, direct ZIP URL and SHA-256.
6. Record verified compatibility and any remaining acceptance limits in `VALIDATION.md`.

Do not submit an architecture to the Plugin List until its package has been validated for the corresponding Notepad++ host architecture.
