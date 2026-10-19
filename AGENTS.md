# 3DShell — Project Rules

## Git

- **Never commit or push unless the user explicitly asks for it in that turn.** The user decides when changes get committed ("yo te digo cuándo hacé commits"). No automatic commits, no proactive `git commit`/`git push`.
- Conventional Commits, no AI attribution.

## Language

- All app UI strings, assets and code comments in **English** — nothing in the app may be in Spanish.

## Architecture (in progress)

- `source/ui/` = reusable UI framework (WinForms-style; must not depend on app code).
- `source/shell/` = 3DShell application code (desktop shell + panel apps + screens).
- UI is transitioning from immediate-mode drawing to retained-mode live controls; system functions go through the app registry (`include/apps.h`).
