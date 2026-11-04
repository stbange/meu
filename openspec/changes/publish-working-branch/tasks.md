## Implementation and verification
- [x] 1. Allow the named working branch in staging/upload/deployment/live-check conditions; retain build gates and least-privilege permissions.
- [x] 2. Serialize runs by workflow/ref and document the publication policy.
- [x] 3. Validate workflow syntax/expressions and active specifications.
- [x] 4. Push the workflow and confirm all four pre-deployment build/test jobs plus Pages staging/upload pass in a real working-branch run.
- [ ] 5. Allow the working branch in the existing github-pages environment and verify deployment plus live browser checks. Blocked by its deployment branch protection rule; connected tools cannot edit the setting.
## Acceptance
- [ ] 6. User confirms the published working build is suitable for their playtest; gameplay acceptance remains pending.
