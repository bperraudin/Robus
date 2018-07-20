Get Travis encrypted credentials
================================

The trickiest part of all this is that you want to give Travis the ability to run your deploy script and push changes to gh-pages, without checking in the necessary credentials to your repo. The solution for this is to use Travis's [encryption support](http://docs.travis-ci.com/user/encryption-keys/).

We'll generate a GitHub personal access token (essentially an application-specific password) and encrypt it, then put the encrypted version in our `.travis.yml` file. Then we can check in the `.travis.yml` file with no issues.

First, generate a token at https://github.com/settings/applications

Then, install the Travis client and do

```
travis encrypt GH_TOKEN=<secret github generated token here>
```

This will give you a very long line like

```
secure: "<.... encrypted data ....>"
```

If you don't want to install Ruby/RubyGems and such, there are reports that the [travis-encrypt](http://npmjs.org/travis-encrypt) npm package works just as well.
